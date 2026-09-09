"""Inspect staged file names and content without printing private matches."""
from pathlib import Path
import json
import re
import subprocess
ROOT=Path(__file__).resolve().parents[1]
def git(*args): return subprocess.check_output(['git',*args],cwd=ROOT)
files=git('diff','--cached','--name-only','--diff-filter=ACMR','-z').decode().split('\0')
private=[]
cfg=ROOT/'settings.local.json'
if cfg.exists(): private=[v for v in json.loads(cfg.read_text(encoding='utf-8-sig')).values() if isinstance(v,str) and len(v)>6]
bad=[]
for name in filter(None,files):
    p=Path(name)
    if p.suffix.lower() in ('.dll','.exe','.uasset','.umap','.bin','.blend','.fbx','.glb','.pdf','.csv') or any(
        part in ('ThirdParty','logs','Saved','Content','artifacts','.venv') for part in p.parts) or '.local.' in name:
        bad.append(name); continue
    text=git('show',':'+name).decode('utf-8')
    if any(v.lower() in text.lower() for v in private) or re.search(r'LHR-[0-9A-Fa-f]{8}|(?:[0-9A-Fa-f]{2}:){5}[0-9A-Fa-f]{2}|[Cc]:[\\/]Users[\\/]',text): bad.append(name)
if bad: raise SystemExit('Public tree check failed: '+', '.join(bad))
print('Public tree check passed:',len([n for n in files if n]),'staged files; no local device IDs or generated binaries.')
