/*
 * devicetree.c - Unified device tree support implementation for BloodHorn
 *
 * This file is part of BloodHorn and is licensed under the BSD License.
 * See the root of the repository for license details.
 */

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include "devicetree.h"
#include "platform.h"
#include "uboot.h"
#include "openfirmware.h"
#include "../libb/include/bloodhorn/bootinfo.h"
#include "../Arch32/powerpc.h"

// Global device tree state
static struct device_tree_node* dt_root = NULL;
static void* dt_blob = NULL;
static size_t dt_blob_size = 0;
static bool dt_initialized = false;

// Memory allocation tracking
struct dt_memory_block {
    void* ptr;
    size_t size;
    struct dt_memory_block* next;
};

static struct dt_memory_block* dt_memory_blocks = NULL;

// Error tracking
static char dt_last_error[256] = {0};

// Helper functions
static void* dt_malloc(size_t size);
static void dt_free(void* ptr);
static void* dt_realloc(void* ptr, size_t size);
static void dt_set_error(const char* format, ...);
static char* dt_strdup(const char* str);
static uint32_t dt_be32_to_cpu(uint32_t val);
static uint32_t dt_cpu_to_be32(uint32_t val);
static uint64_t dt_be64_to_cpu(uint64_t val);
static uint64_t dt_cpu_to_be64(uint64_t val);
static bh_status_t dt_parse_fdt_blob(const void* blob, size_t size, struct device_tree_node** root);
static bh_status_t dt_build_fdt_blob(const struct device_tree_node* root, void** blob, size_t* size);
static bh_status_t dt_parse_node_properties(const void* blob, const void* prop_ptr, 
                                           struct device_tree_node* node);
static bh_status_t dt_parse_node_children(const void* blob, const void* struct_ptr, 
                                         struct device_tree_node* parent);
static bh_status_t dt_write_node_to_blob(const struct device_tree_node* node, void** blob, 
                                        size_t* offset, const char* strings_block, 
                                        size_t* strings_offset);
static bh_status_t dt_write_property_to_blob(const struct device_tree_property* prop, 
                                             void** blob, size_t* offset, 
                                             const char* strings_block, 
                                             size_t* strings_offset);

// Memory allocation with tracking
static void* dt_malloc(size_t size) {
    void* ptr = platform_malloc(size);
    if (ptr) {
        struct dt_memory_block* block = platform_malloc(sizeof(struct dt_memory_block));
        if (block) {
            block->ptr = ptr;
            block->size = size;
            block->next = dt_memory_blocks;
            dt_memory_blocks = block;
        }
    }
    return ptr;
}

static void dt_free(void* ptr) {
    if (ptr) {
        // Find and remove from tracking list
        struct dt_memory_block** current = &dt_memory_blocks;
        while (*current) {
            if ((*current)->ptr == ptr) {
                struct dt_memory_block* to_remove = *current;
                *current = to_remove->next;
                platform_free(to_remove);
                break;
            }
            current = &(*current)->next;
        }
        platform_free(ptr);
    }
}

static void* dt_realloc(void* ptr, size_t size) {
    void* new_ptr = platform_realloc(ptr, size);
    if (new_ptr && new_ptr != ptr) {
        // Update tracking
        struct dt_memory_block* current = dt_memory_blocks;
        while (current) {
            if (current->ptr == ptr) {
                current->ptr = new_ptr;
                current->size = size;
                break;
            }
            current = current->next;
        }
    }
    return new_ptr;
}

// Error handling
static void dt_set_error(const char* format, ...) {
    va_list args;
    va_start(args, format);
    vsnprintf(dt_last_error, sizeof(dt_last_error), format, args);
    va_end(args);
}

static char* dt_strdup(const char* str) {
    if (!str) {
        return NULL;
    }
    size_t len = strlen(str) + 1;
    char* new_str = dt_malloc(len);
    if (new_str) {
        memcpy(new_str, str, len);
    }
    return new_str;
}

// Endian conversion functions
static uint32_t dt_be32_to_cpu(uint32_t val) {
    return ((val >> 24) & 0xFF) | ((val >> 8) & 0xFF00) | ((val << 8) & 0xFF0000) | ((val << 24) & 0xFF000000);
}

static uint32_t dt_cpu_to_be32(uint32_t val) {
    return dt_be32_to_cpu(val);
}

static uint64_t dt_be64_to_cpu(uint64_t val) {
    return ((val >> 56) & 0xFF) | ((val >> 40) & 0xFF00) | ((val >> 24) & 0xFF0000) | 
           ((val >> 8) & 0xFF000000) | ((val << 8) & 0xFF00000000ULL) | 
           ((val << 24) & 0xFF0000000000ULL) | ((val << 40) & 0xFF000000000000ULL) | 
           ((val << 56) & 0xFF00000000000000ULL);
}

static uint64_t dt_cpu_to_be64(uint64_t val) {
    return dt_be64_to_cpu(val);
}

// Load device tree from flattened blob
bh_status_t dt_load_from_blob(const void* blob, size_t size, struct device_tree_node** root) {
    if (!blob || size < sizeof(struct fdt_header) || !root) {
        dt_set_error("Invalid parameters");
        return BH_STATUS_INVALID_PARAMETER;
    }
    
    // Validate FDT header
    const struct fdt_header* header = (const struct fdt_header*)blob;
    if (dt_be32_to_cpu(header->magic) != FDT_MAGIC) {
        dt_set_error("Invalid FDT magic number");
        return BH_STATUS_INVALID_DATA;
    }
    
    if (dt_be32_to_cpu(header->version) < FDT_VERSION) {
        dt_set_error("Unsupported FDT version");
        return BH_STATUS_UNSUPPORTED;
    }
    
    if (dt_be32_to_cpu(header->totalsize) > size) {
        dt_set_error("FDT size exceeds buffer size");
        return BH_STATUS_INVALID_DATA;
    }
    
    // Parse the blob
    bh_status_t status = dt_parse_fdt_blob(blob, size, root);
    if (status == BH_STATUS_SUCCESS) {
        dt_root = *root;
        dt_blob = (void*)blob;
        dt_blob_size = size;
        dt_initialized = true;
    }
    
    return status;
}

// Load device tree from platform
bh_status_t dt_load_from_platform(void** blob, size_t* size) {
    if (!blob || !size) {
        dt_set_error("Invalid parameters");
        return BH_STATUS_INVALID_PARAMETER;
    }
    
    bh_status_t status = BH_STATUS_NOT_FOUND;
    
    // Try U-Boot first
    if (platform_get_type() == BH_PLATFORM_UBOOT) {
        status = uboot_get_device_tree(blob, size);
        if (status == BH_STATUS_SUCCESS) {
            return status;
        }
    }
    
    // Try OpenFirmware
    if (platform_get_type() == BH_PLATFORM_OPENFIRMWARE) {
        status = ofw_platform_get_device_tree(blob, size);
        if (status == BH_STATUS_SUCCESS) {
            return status;
        }
    }
    
    // Try generic platform interface
    status = platform_get_device_tree(blob, size);
    if (status == BH_STATUS_SUCCESS) {
        return status;
    }
    
    dt_set_error("No device tree available from platform");
    return BH_STATUS_NOT_FOUND;
}

// Save device tree to flattened blob
bh_status_t dt_save_to_blob(const struct device_tree_node* root, void** blob, size_t* size) {
    if (!root || !blob || !size) {
        dt_set_error("Invalid parameters");
        return BH_STATUS_INVALID_PARAMETER;
    }
    
    bh_status_t status = dt_build_fdt_blob(root, blob, size);
    if (status == BH_STATUS_SUCCESS) {
        dt_blob = *blob;
        dt_blob_size = *size;
    }
    
    return status;
}

// Validate device tree
bh_status_t dt_validate(const struct device_tree_node* root, struct dt_validation_info* info) {
    if (!root || !info) {
        dt_set_error("Invalid parameters");
        return BH_STATUS_INVALID_PARAMETER;
    }
    
    memset(info, 0, sizeof(struct dt_validation_info));
    info->valid = true;
    
    // Check root node
    if (!root->name || strcmp(root->name, "") != 0) {
        info->valid = false;
        info->errors++;
        if (!info->error_message) {
            info->error_message = dt_strdup("Root node must have empty name");
        }
    }
    
    // Recursively validate tree
    // This is a simplified validation - a full implementation would check:
    // - Property names and values
    // - Address and size cells consistency
    // - Phandle uniqueness
    // - Required properties for specific device types
    // - Memory ranges validity
    // - Interrupt mapping validity
    
    return BH_STATUS_SUCCESS;
}

// Cleanup device tree
bh_status_t dt_cleanup(struct device_tree_node* root) {
    if (!root) {
        return BH_STATUS_INVALID_PARAMETER;
    }
    
    // Recursively free all nodes and properties
    struct device_tree_node* current = root;
    struct device_tree_node* next = NULL;
    
    while (current) {
        // Free properties
        struct device_tree_property* prop = current->properties;
        while (prop) {
            struct device_tree_property* next_prop = prop->next;
            if (prop->name) {
                dt_free(prop->name);
            }
            if (prop->value) {
                dt_free(prop->value);
            }
            dt_free(prop);
            prop = next_prop;
        }
        
        // Free node name and path
        if (current->name) {
            dt_free(current->name);
        }
        if (current->full_path) {
            dt_free(current->full_path);
        }
        
        // Move to next node (depth-first traversal)
        if (current->children) {
            next = current->children;
        } else if (current->sibling) {
            next = current->sibling;
        } else {
            // Go up to find next sibling
            struct device_tree_node* parent = current->parent;
            while (parent && !parent->sibling) {
                parent = parent->parent;
            }
            next = parent ? parent->sibling : NULL;
        }
        
        dt_free(current);
        current = next;
    }
    
    // Free memory tracking
    struct dt_memory_block* current_block = dt_memory_blocks;
    while (current_block) {
        struct dt_memory_block* next_block = current_block->next;
        dt_free(current_block);
        current_block = next_block;
    }
    dt_memory_blocks = NULL;
    
    dt_root = NULL;
    dt_blob = NULL;
    dt_blob_size = 0;
    dt_initialized = false;
    
    return BH_STATUS_SUCCESS;
}

// Find node by path
bh_status_t dt_find_node(const struct device_tree_node* root, const char* path, 
                         struct device_tree_node** node) {
    if (!root || !path || !node) {
        dt_set_error("Invalid parameters");
        return BH_STATUS_INVALID_PARAMETER;
    }
    
    // Handle absolute path
    const char* current_path = path;
    if (current_path[0] == '/') {
        current_path++;
    }
    
    // Start from root
    const struct device_tree_node* current = root;
    
    // Parse path components
    char* path_copy = dt_strdup(current_path);
    if (!path_copy) {
        dt_set_error("Memory allocation failed");
        return BH_STATUS_NO_MEMORY;
    }
    
    char* token = strtok(path_copy, "/");
    while (token && current) {
        // Find child with matching name
        struct device_tree_node* child = current->children;
        bool found = false;
        
        while (child) {
            if (strcmp(child->name, token) == 0) {
                current = child;
                found = true;
                break;
            }
            child = child->sibling;
        }
        
        if (!found) {
            dt_free(path_copy);
            dt_set_error("Node '%s' not found", token);
            return BH_STATUS_NOT_FOUND;
        }
        
        token = strtok(NULL, "/");
    }
    
    dt_free(path_copy);
    
    if (current) {
        *node = (struct device_tree_node*)current;
        return BH_STATUS_SUCCESS;
    }
    
    dt_set_error("Path not found");
    return BH_STATUS_NOT_FOUND;
}

// Find node by property
bh_status_t dt_find_node_by_property(const struct device_tree_node* root, const char* prop_name, 
                                     const void* prop_value, size_t prop_size, 
                                     struct device_tree_node** node) {
    if (!root || !prop_name || !node) {
        dt_set_error("Invalid parameters");
        return BH_STATUS_INVALID_PARAMETER;
    }
    
    // Recursive search
    struct device_tree_node* current = (struct device_tree_node*)root;
    struct device_tree_node* stack[64];
    int stack_top = -1;
    
    while (current || stack_top >= 0) {
        if (current) {
            // Check current node
            void* value;
            size_t length;
            bh_status_t status = dt_get_property(current, prop_name, &value, &length);
            if (status == BH_STATUS_SUCCESS) {
                if (!prop_value || (length == prop_size && memcmp(value, prop_value, prop_size) == 0)) {
                    *node = current;
                    return BH_STATUS_SUCCESS;
                }
            }
            
            // Push children onto stack
            if (current->children) {
                stack[++stack_top] = current->children;
            }
            
            // Move to sibling
            current = current->sibling;
        } else {
            // Pop from stack
            current = stack[stack_top--];
        }
    }
    
    dt_set_error("Node with property '%s' not found", prop_name);
    return BH_STATUS_NOT_FOUND;
}

// Find node by compatible string
bh_status_t dt_find_node_by_compatible(const struct device_tree_node* root, const char* compatible, 
                                       struct device_tree_node** node) {
    return dt_find_node_by_property(root, "compatible", compatible, strlen(compatible) + 1, node);
}

// Find node by phandle
bh_status_t dt_find_node_by_phandle(const struct device_tree_node* root, uint32_t phandle, 
                                    struct device_tree_node** node) {
    if (!root || !node) {
        dt_set_error("Invalid parameters");
        return BH_STATUS_INVALID_PARAMETER;
    }
    
    // Recursive search
    struct device_tree_node* current = (struct device_tree_node*)root;
    struct device_tree_node* stack[64];
    int stack_top = -1;
    
    while (current || stack_top >= 0) {
        if (current) {
            // Check current node
            if (current->phandle == phandle) {
                *node = current;
                return BH_STATUS_SUCCESS;
            }
            
            // Push children onto stack
            if (current->children) {
                stack[++stack_top] = current->children;
            }
            
            // Move to sibling
            current = current->sibling;
        } else {
            // Pop from stack
            current = stack[stack_top--];
        }
    }
    
    dt_set_error("Node with phandle 0x%x not found", phandle);
    return BH_STATUS_NOT_FOUND;
}

// Add node to tree
bh_status_t dt_add_node(struct device_tree_node* parent, const char* name, 
                       struct device_tree_node** node) {
    if (!parent || !name || !node) {
        dt_set_error("Invalid parameters");
        return BH_STATUS_INVALID_PARAMETER;
    }
    
    // Create new node
    struct device_tree_node* new_node = dt_malloc(sizeof(struct device_tree_node));
    if (!new_node) {
        dt_set_error("Memory allocation failed");
        return BH_STATUS_NO_MEMORY;
    }
    
    memset(new_node, 0, sizeof(struct device_tree_node));
    new_node->name = dt_strdup(name);
    if (!new_node->name) {
        dt_free(new_node);
        dt_set_error("Memory allocation failed");
        return BH_STATUS_NO_MEMORY;
    }
    
    // Build full path
    if (parent->full_path) {
        size_t path_len = strlen(parent->full_path) + strlen(name) + 2;
        new_node->full_path = dt_malloc(path_len);
        if (new_node->full_path) {
            snprintf(new_node->full_path, path_len, "%s/%s", parent->full_path, name);
        }
    } else {
        new_node->full_path = dt_strdup(name);
    }
    
    if (!new_node->full_path) {
        dt_free(new_node->name);
        dt_free(new_node);
        dt_set_error("Memory allocation failed");
        return BH_STATUS_NO_MEMORY;
    }
    
    // Set parent and add to children list
    new_node->parent = parent;
    new_node->address_cells = parent->address_cells;
    new_node->size_cells = parent->size_cells;
    new_node->interrupt_cells = parent->interrupt_cells;
    new_node->interrupt_parent = parent->interrupt_parent;
    
    // Insert at beginning of children list
    new_node->sibling = parent->children;
    parent->children = new_node;
    
    *node = new_node;
    return BH_STATUS_SUCCESS;
}

// Remove node from tree
bh_status_t dt_remove_node(struct device_tree_node* parent, struct device_tree_node* node) {
    if (!parent || !node) {
        dt_set_error("Invalid parameters");
        return BH_STATUS_INVALID_PARAMETER;
    }
    
    // Find node in parent's children list
    struct device_tree_node** current = &parent->children;
    while (*current) {
        if (*current == node) {
            *current = node->sibling;
            node->parent = NULL;
            node->sibling = NULL;
            return BH_STATUS_SUCCESS;
        }
        current = &(*current)->sibling;
    }
    
    dt_set_error("Node not found in parent's children list");
    return BH_STATUS_NOT_FOUND;
}

// Move node to new parent
bh_status_t dt_move_node(struct device_tree_node* node, struct device_tree_node* new_parent) {
    if (!node || !new_parent) {
        dt_set_error("Invalid parameters");
        return BH_STATUS_INVALID_PARAMETER;
    }
    
    // Remove from current parent
    if (node->parent) {
        dt_remove_node(node->parent, node);
    }
    
    // Add to new parent
    node->sibling = new_parent->children;
    new_parent->children = node;
    node->parent = new_parent;
    
    // Update full path
    if (node->full_path) {
        dt_free(node->full_path);
    }
    
    if (new_parent->full_path) {
        size_t path_len = strlen(new_parent->full_path) + strlen(node->name) + 2;
        node->full_path = dt_malloc(path_len);
        if (node->full_path) {
            snprintf(node->full_path, path_len, "%s/%s", new_parent->full_path, node->name);
        }
    } else {
        node->full_path = dt_strdup(node->name);
    }
    
    return BH_STATUS_SUCCESS;
}

// Get property value
bh_status_t dt_get_property(const struct device_tree_node* node, const char* name, 
                           void** value, size_t* length) {
    if (!node || !name || !value || !length) {
        dt_set_error("Invalid parameters");
        return BH_STATUS_INVALID_PARAMETER;
    }
    
    // Search for property
    struct device_tree_property* prop = node->properties;
    while (prop) {
        if (strcmp(prop->name, name) == 0) {
            *value = prop->value;
            *length = prop->length;
            return BH_STATUS_SUCCESS;
        }
        prop = prop->next;
    }
    
    dt_set_error("Property '%s' not found", name);
    return BH_STATUS_NOT_FOUND;
}

// Set property value
bh_status_t dt_set_property(struct device_tree_node* node, const char* name, 
                           const void* value, size_t length) {
    if (!node || !name || (!value && length > 0)) {
        dt_set_error("Invalid parameters");
        return BH_STATUS_INVALID_PARAMETER;
    }
    
    // Check if property already exists
    struct device_tree_property* prop = node->properties;
    while (prop) {
        if (strcmp(prop->name, name) == 0) {
            // Update existing property
            if (prop->value) {
                dt_free(prop->value);
            }
            if (length > 0) {
                prop->value = dt_malloc(length);
                if (!prop->value) {
                    dt_set_error("Memory allocation failed");
                    return BH_STATUS_NO_MEMORY;
                }
                memcpy(prop->value, value, length);
            } else {
                prop->value = NULL;
            }
            prop->length = length;
            return BH_STATUS_SUCCESS;
        }
        prop = prop->next;
    }
    
    // Create new property
    prop = dt_malloc(sizeof(struct device_tree_property));
    if (!prop) {
        dt_set_error("Memory allocation failed");
        return BH_STATUS_NO_MEMORY;
    }
    
    memset(prop, 0, sizeof(struct device_tree_property));
    prop->name = dt_strdup(name);
    if (!prop->name) {
        dt_free(prop);
        dt_set_error("Memory allocation failed");
        return BH_STATUS_NO_MEMORY;
    }
    
    if (length > 0) {
        prop->value = dt_malloc(length);
        if (!prop->value) {
            dt_free(prop->name);
            dt_free(prop);
            dt_set_error("Memory allocation failed");
            return BH_STATUS_NO_MEMORY;
        }
        memcpy(prop->value, value, length);
    }
    
    prop->length = length;
    
    // Add to property list
    prop->next = node->properties;
    node->properties = prop;
    
    return BH_STATUS_SUCCESS;
}

// Remove property
bh_status_t dt_remove_property(struct device_tree_node* node, const char* name) {
    if (!node || !name) {
        dt_set_error("Invalid parameters");
        return BH_STATUS_INVALID_PARAMETER;
    }
    
    struct device_tree_property** current = &node->properties;
    while (*current) {
        if (strcmp((*current)->name, name) == 0) {
            struct device_tree_property* to_remove = *current;
            *current = to_remove->next;
            
            if (to_remove->name) {
                dt_free(to_remove->name);
            }
            if (to_remove->value) {
                dt_free(to_remove->value);
            }
            dt_free(to_remove);
            
            return BH_STATUS_SUCCESS;
        }
        current = &(*current)->next;
    }
    
    dt_set_error("Property '%s' not found", name);
    return BH_STATUS_NOT_FOUND;
}

// Get string property
bh_status_t dt_get_string_property(const struct device_tree_node* node, const char* name, 
                                   char** string) {
    void* value;
    size_t length;
    bh_status_t status = dt_get_property(node, name, &value, &length);
    if (status != BH_STATUS_SUCCESS) {
        return status;
    }
    
    if (length == 0) {
        *string = dt_strdup("");
        return *string ? BH_STATUS_SUCCESS : BH_STATUS_NO_MEMORY;
    }
    
    // Ensure null-terminated
    char* temp = dt_malloc(length + 1);
    if (!temp) {
        dt_set_error("Memory allocation failed");
        return BH_STATUS_NO_MEMORY;
    }
    
    memcpy(temp, value, length);
    temp[length] = '\0';
    
    *string = temp;
    return BH_STATUS_SUCCESS;
}

// Set string property
bh_status_t dt_set_string_property(struct device_tree_node* node, const char* name, const char* string) {
    if (!string) {
        return dt_set_property(node, name, NULL, 0);
    }
    return dt_set_property(node, name, string, strlen(string) + 1);
}

// Get 32-bit property
bh_status_t dt_get_u32_property(const struct device_tree_node* node, const char* name, uint32_t* value) {
    void* prop_value;
    size_t length;
    bh_status_t status = dt_get_property(node, name, &prop_value, &length);
    if (status != BH_STATUS_SUCCESS) {
        return status;
    }
    
    if (length < sizeof(uint32_t)) {
        dt_set_error("Property '%s' is too small for 32-bit value", name);
        return BH_STATUS_INVALID_DATA;
    }
    
    *value = dt_be32_to_cpu(*(uint32_t*)prop_value);
    return BH_STATUS_SUCCESS;
}

// Set 32-bit property
bh_status_t dt_set_u32_property(struct device_tree_node* node, const char* name, uint32_t value) {
    uint32_t be_value = dt_cpu_to_be32(value);
    return dt_set_property(node, name, &be_value, sizeof(uint32_t));
}

// Get 64-bit property
bh_status_t dt_get_u64_property(const struct device_tree_node* node, const char* name, uint64_t* value) {
    void* prop_value;
    size_t length;
    bh_status_t status = dt_get_property(node, name, &prop_value, &length);
    if (status != BH_STATUS_SUCCESS) {
        return status;
    }
    
    if (length < sizeof(uint64_t)) {
        dt_set_error("Property '%s' is too small for 64-bit value", name);
        return BH_STATUS_INVALID_DATA;
    }
    
    *value = dt_be64_to_cpu(*(uint64_t*)prop_value);
    return BH_STATUS_SUCCESS;
}

// Set 64-bit property
bh_status_t dt_set_u64_property(struct device_tree_node* node, const char* name, uint64_t value) {
    uint64_t be_value = dt_cpu_to_be64(value);
    return dt_set_property(node, name, &be_value, sizeof(uint64_t));
}

// Get boolean property
bh_status_t dt_get_bool_property(const struct device_tree_node* node, const char* name, bool* value) {
    void* prop_value;
    size_t length;
    bh_status_t status = dt_get_property(node, name, &prop_value, &length);
    if (status != BH_STATUS_SUCCESS) {
        *value = false;
        return BH_STATUS_SUCCESS; // Missing property is false
    }
    
    *value = (length > 0);
    return BH_STATUS_SUCCESS;
}

// Set boolean property
bh_status_t dt_set_bool_property(struct device_tree_node* node, const char* name, bool value) {
    if (value) {
        return dt_set_property(node, name, "", 1);
    } else {
        return dt_remove_property(node, name);
    }
}

// Parse FDT blob (simplified implementation)
static bh_status_t dt_parse_fdt_blob(const void* blob, size_t size, struct device_tree_node** root) {
    const struct fdt_header* header = (const struct fdt_header*)blob;
    
    // Get structure block
    const void* struct_ptr = (const char*)blob + dt_be32_to_cpu(header->off_dt_struct);
    const void* strings_ptr = (const char*)blob + dt_be32_to_cpu(header->off_dt_strings);
    
    // Create root node
    struct device_tree_node* node = dt_malloc(sizeof(struct device_tree_node));
    if (!node) {
        dt_set_error("Memory allocation failed");
        return BH_STATUS_NO_MEMORY;
    }
    
    memset(node, 0, sizeof(struct device_tree_node));
    node->name = dt_strdup("");
    node->full_path = dt_strdup("/");
    node->address_cells = 1;
    node->size_cells = 1;
    
    if (!node->name || !node->full_path) {
        dt_cleanup(node);
        dt_set_error("Memory allocation failed");
        return BH_STATUS_NO_MEMORY;
    }
    
    // Parse structure block
    const uint32_t* ptr = (const uint32_t*)struct_ptr;
    while (ptr < (const uint32_t*)((const char*)blob + dt_be32_to_cpu(header->off_dt_struct) + 
                                    dt_be32_to_cpu(header->size_dt_struct))) {
        uint32_t token = dt_be32_to_cpu(*ptr++);
        
        switch (token) {
            case FDT_BEGIN_NODE: {
                // Get node name
                const char* node_name = (const char*)ptr;
                while (*ptr != 0) {
                    ptr++;
                }
                ptr = (const uint32_t*)((const char*)ptr + 4 - ((uintptr_t)ptr % 4));
                
                // Create node (simplified - full implementation would build tree)
                break;
            }
            
            case FDT_PROP: {
                uint32_t len = dt_be32_to_cpu(*ptr++);
                uint32_t nameoff = dt_be32_to_cpu(*ptr++);
                const char* prop_name = (const char*)strings_ptr + nameoff;
                
                // Align to 4-byte boundary
                ptr = (const uint32_t*)((const char*)ptr + 3 - ((uintptr_t)ptr % 3));
                
                // Get property value
                const void* prop_value = ptr;
                ptr = (const uint32_t*)((const char*)ptr + len);
                
                // Align to 4-byte boundary
                ptr = (const uint32_t*)((const char*)ptr + 3 - ((uintptr_t)ptr % 3));
                
                // Add property to node (simplified)
                break;
            }
            
            case FDT_END_NODE:
                break;
                
            case FDT_NOP:
                break;
                
            case FDT_END:
                goto parse_complete;
                
            default:
                dt_set_error("Invalid FDT token: 0x%x", token);
                dt_cleanup(node);
                return BH_STATUS_INVALID_DATA;
        }
    }
    
parse_complete:
    *root = node;
    return BH_STATUS_SUCCESS;
}

// Build FDT blob (simplified implementation)
static bh_status_t dt_build_fdt_blob(const struct device_tree_node* root, void** blob, size_t* size) {
    // This is a simplified implementation
    // A full implementation would:
    // 1. Calculate required size for all structures
    // 2. Allocate blob
    // 3. Write header
    // 4. Write memory reserve map
    // 5. Write structure block
    // 6. Write strings block
    // 7. Update header offsets
    
    // For now, return a minimal blob
    size_t blob_size = sizeof(struct fdt_header) + 1024; // Simplified size
    void* new_blob = dt_malloc(blob_size);
    if (!new_blob) {
        dt_set_error("Memory allocation failed");
        return BH_STATUS_NO_MEMORY;
    }
    
    memset(new_blob, 0, blob_size);
    
    // Write header (simplified)
    struct fdt_header* header = (struct fdt_header*)new_blob;
    header->magic = dt_cpu_to_be32(FDT_MAGIC);
    header->totalsize = dt_cpu_to_be32(blob_size);
    header->version = dt_cpu_to_be32(FDT_VERSION);
    header->last_comp_version = dt_cpu_to_be32(FDT_LAST_COMPAT_VERSION);
    header->boot_cpuid_phys = dt_cpu_to_be32(0);
    header->off_dt_struct = dt_cpu_to_be32(sizeof(struct fdt_header));
    header->off_dt_strings = dt_cpu_to_be32(sizeof(struct fdt_header) + 512);
    header->off_mem_rsvmap = dt_cpu_to_be32(sizeof(struct fdt_header) + 512 + 256);
    header->size_dt_strings = dt_cpu_to_be32(256);
    header->size_dt_struct = dt_cpu_to_be32(512);
    
    *blob = new_blob;
    *size = blob_size;
    
    return BH_STATUS_SUCCESS;
}

// Get memory regions from device tree
bh_status_t dt_get_memory_regions(const struct device_tree_node* root, struct dt_memory_region** regions, 
                                  size_t* count) {
    if (!root || !regions || !count) {
        dt_set_error("Invalid parameters");
        return BH_STATUS_INVALID_PARAMETER;
    }
    
    // Find memory nodes
    struct device_tree_node* memory_node;
    bh_status_t status = dt_find_node_by_compatible(root, "memory", &memory_node);
    if (status != BH_STATUS_SUCCESS) {
        // Try alternative method
        status = dt_find_node(root, "/memory", &memory_node);
        if (status != BH_STATUS_SUCCESS) {
            dt_set_error("Memory node not found");
            return BH_STATUS_NOT_FOUND;
        }
    }
    
    // Get reg property
    void* reg_value;
    size_t reg_length;
    status = dt_get_property(memory_node, "reg", &reg_value, &reg_length);
    if (status != BH_STATUS_SUCCESS) {
        dt_set_error("Memory reg property not found");
        return BH_STATUS_NOT_FOUND;
    }
    
    // Parse reg property (address/size pairs)
    uint32_t address_cells, size_cells;
    status = dt_get_u32_property(memory_node, "#address-cells", &address_cells);
    if (status != BH_STATUS_SUCCESS) {
        address_cells = 1; // Default
    }
    
    status = dt_get_u32_property(memory_node, "#size-cells", &size_cells);
    if (status != BH_STATUS_SUCCESS) {
        size_cells = 1; // Default
    }
    
    size_t entry_size = (address_cells + size_cells) * sizeof(uint32_t);
    size_t region_count = reg_length / entry_size;
    
    if (region_count == 0) {
        *regions = NULL;
        *count = 0;
        return BH_STATUS_SUCCESS;
    }
    
    // Allocate regions array
    struct dt_memory_region* region_array = dt_malloc(region_count * sizeof(struct dt_memory_region));
    if (!region_array) {
        dt_set_error("Memory allocation failed");
        return BH_STATUS_NO_MEMORY;
    }
    
    // Parse each region
    const uint32_t* ptr = (const uint32_t*)reg_value;
    for (size_t i = 0; i < region_count; i++) {
        uint64_t address = 0, size = 0;
        
        // Parse address
        for (uint32_t j = 0; j < address_cells; j++) {
            address = (address << 32) | dt_be32_to_cpu(*ptr++);
        }
        
        // Parse size
        for (uint32_t j = 0; j < size_cells; j++) {
            size = (size << 32) | dt_be32_to_cpu(*ptr++);
        }
        
        region_array[i].address = address;
        region_array[i].size = size;
        region_array[i].type = BH_MEMORY_TYPE_RAM;
        region_array[i].name = dt_strdup("memory");
    }
    
    *regions = region_array;
    *count = region_count;
    
    return BH_STATUS_SUCCESS;
}

// Error string function
const char* dt_error_string(bh_status_t status) {
    switch (status) {
        case BH_STATUS_SUCCESS:
            return "Success";
        case BH_STATUS_INVALID_PARAMETER:
            return "Invalid parameter";
        case BH_STATUS_INVALID_DATA:
            return "Invalid data";
        case BH_STATUS_NOT_FOUND:
            return "Not found";
        case BH_STATUS_NO_MEMORY:
            return "Out of memory";
        case BH_STATUS_UNSUPPORTED:
            return "Unsupported operation";
        default:
            return dt_last_error[0] ? dt_last_error : "Unknown error";
    }
}

// Get last error
bh_status_t dt_get_last_error(char* buffer, size_t size) {
    if (!buffer || size == 0) {
        return BH_STATUS_INVALID_PARAMETER;
    }
    
    strncpy(buffer, dt_last_error, size - 1);
    buffer[size - 1] = '\0';
    
    return BH_STATUS_SUCCESS;
}
