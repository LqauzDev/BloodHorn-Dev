#!/usr/bin/env bash
set -euo pipefail

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
inf_file="$repo_root/BloodHorn.inf"
dsc_file="$repo_root/BloodHorn.dsc"

for f in "$inf_file" "$dsc_file"; do
  [[ -f "$f" ]] || { echo "ERROR: required file missing: $f" >&2; exit 1; }
done

missing=0
while IFS= read -r src; do
  [[ -z "$src" ]] && continue
  [[ -f "$repo_root/$src" ]] || { echo "ERROR: INF source missing: $src" >&2; missing=1; }
done < <(awk 'BEGIN{s=0} /^\[Sources\]/{s=1;next} /^\[/{if(s) exit} s && $1 !~ /^#/ && NF {print $1}' "$inf_file")

if [[ $missing -ne 0 ]]; then
  exit 1
fi

echo "EDK2 project layout check passed."
