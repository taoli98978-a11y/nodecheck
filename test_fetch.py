"""Test harness transport only. The Windows EXE never launches this script."""
import hashlib,json,sys,time,urllib.request,urllib.error
from pathlib import Path
url=bytes.fromhex(sys.argv[1]).decode();root=Path(__file__).parent/'test-cache';root.mkdir(exist_ok=True)
name=hashlib.sha256(url.encode()).hexdigest();datafile=root/(name+'.body');meta=root/(name+'.json')
if datafile.exists() and meta.exists():
 m=json.loads(meta.read_text());status=m['status'];body=datafile.read_bytes()
else:
 try:
  with urllib.request.urlopen(urllib.request.Request(url,headers={'User-Agent':'NodeCheck/1.5 (interactive postal lookup)'}),timeout=8) as r:status=r.status;body=r.read(2097152)
 except urllib.error.HTTPError as e:status=e.code;body=e.read(2097152)
 except Exception:status=0;body=b''
 datafile.write_bytes(body);meta.write_text(json.dumps({'url':url,'status':status,'fetched':time.time()}))
sys.stdout.buffer.write(str(status).encode()+b'\n'+body)
