#!/usr/bin/env python3
import subprocess
import re

BASE_VERSION = "0.0.0"

def parse_version(v):
    major, minor, patch = map(int, v.split("."))
    return major, minor, patch

def bump_version(version, level):
    major, minor, patch = parse_version(version)
    if level == "major":
        major += 1
        minor = 0
        patch = 0
    elif level == "minor":
        minor += 1
        patch = 0
    else:
        patch += 1
    return f"{major}.{minor}.{patch}"

def get_commits():
    # Get all commits in chronological order
    result = subprocess.check_output(
        ["git", "rev-list", "--reverse", "HEAD"], text=True
    )
    return result.strip().splitlines()

def get_commit_message(commit):
    msg = subprocess.check_output(
        ["git", "log", "-1", "--pretty=%B", commit], text=True
    )
    return msg.strip()

def rewrite_commit_message(commit, new_msg):
    # Use git filter-branch to rewrite message
    subprocess.run([
        "git", "filter-branch", "-f",
        "--msg-filter", f'echo "{new_msg}"',
        "--", commit
    ], shell=True)

def main():
    commits = get_commits()
    version = BASE_VERSION

    for c in commits:
        msg = get_commit_message(c)
        if "[major]" in msg.lower():
            level = "major"
        elif "[minor]" in msg.lower():
            level = "minor"
        else:
            level = "patch"
        version = bump_version(version, level)

        # Optionally append version to commit message
        new_msg = f"[v{version}] {msg}"
        print(f"{c[:7]} -> {version} -> {msg}")
        # Uncomment below to actually rewrite commit messages (destructive)
        # rewrite_commit_message(c, new_msg)

        # Optionally tag commit
        subprocess.run(["git", "tag", f"v{version}", c], shell=True)

if __name__ == "__main__":
    main()
