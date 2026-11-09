"""Refresh GeoNames US postal candidates. No API key. License: CC BY 4.0."""
import argparse,io,json,zipfile,urllib.request,hashlib
from pathlib import Path
p=argparse.ArgumentParser();p.add_argument('--zip',type=Path);a=p.parse_args()
data=a.zip.read_bytes() if a.zip else urllib.request.urlopen('https://download.geonames.org/export/zip/US.zip',timeout=60).read()
z=zipfile.ZipFile(io.BytesIO(data));records=[]
for line in z.read('US.txt').decode('utf-8').splitlines():
 r=line.split('\t')
 if len(r)<12 or len(r[1])!=5 or not r[1].isdigit() or len(r[4])!=2:continue
 if len(r[2].encode())>63:raise ValueError('city name exceeds field')
 records.append((r[1],r[4],r[2],float(r[9]),float(r[10])))
root=Path(__file__).resolve().parent
with (root/'postal_data.h').open('w') as f:
 f.write('// Derived from GeoNames US.zip; CC BY 4.0. See THIRD_PARTY.txt.\n')
 f.write('typedef struct {char zip[6],state[3],city[64];float lat,lon;} Postal;\nstatic const Postal postalData[]={\n')
 for zip_,state,city,lat,lon in records:f.write('{'+','.join([json.dumps(zip_),json.dumps(state),json.dumps(city),f'{lat:.6f}f',f'{lon:.6f}f'])+'},\n')
 f.write('};\n')
(root/'THIRD_PARTY.txt').write_text('GeoNames postal candidates\nhttps://www.geonames.org/\nhttps://download.geonames.org/export/zip/US.zip\nLicense: Creative Commons Attribution 4.0, https://creativecommons.org/licenses/by/4.0/\nRetrieved: 2026-11-01. Derived fields: ZIP, state code, city, latitude, longitude. Not residential evidence. Coordinates are approximate postal representative points.\nSHA256 of source ZIP: '+hashlib.sha256(data).hexdigest()+'\nRows: '+str(len(records))+'\n\n'+z.read('readme.txt').decode(),encoding='utf-8')
print(len(records),'postal candidates written')
