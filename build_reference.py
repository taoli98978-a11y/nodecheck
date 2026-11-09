"""Build redistributable offline references. Requires only Python standard library.
Use --refresh to fetch published sources; default rebuild uses bundled CSVs.
"""
import argparse,csv,hashlib,io,json,pathlib,urllib.request
ROOT=pathlib.Path(__file__).resolve().parent
URLS={
 'zip_reference.csv':'https://zipcodes.pro/download/us-zip-codes.csv',
 'zip_LICENSE.txt':'https://zipcodes.pro/download/LICENSE.txt',
 'zip_SOURCES.csv':'https://zipcodes.pro/download/SOURCES.csv',
 'housing_full.dat':'https://www2.census.gov/programs-surveys/acs/summary_file/2024/table-based-SF/data/5YRData/acsdt5y2024-b25003.dat'}
def build():
 rows=list(csv.DictReader((ROOT/'zip_reference.csv').open()))
 housing={r['zip']:r for r in csv.DictReader((ROOT/'housing_2024.csv').open())}
 lines=['// Derived reference data. Attribution and dates: THIRD_PARTY.txt',
 'typedef struct {char zip[6],state[3],city[80],area[80];float lat,lon;int occupied,moe,standard;} LocalPostal;',
 'static const LocalPostal localPostal[]={']
 count=0
 for r in rows:
  if not r['lat'] or not r['lng'] or r['state'] in ('AA','AE','AP'):continue
  h=housing.get(r['zip'],{});occupied=int(h.get('occupied','-1'));moe=int(h.get('moe','-1'))
  assert len(r['city'].encode())<80 and len(r['area_codes'])<80
  args=[json.dumps(r[k],ensure_ascii=False) for k in ['zip','state','city','area_codes']]
  args += [f"{float(r['lat']):.6f}f",f"{float(r['lng']):.6f}f",str(occupied),str(moe),str(int(r['type']=='standard' and r['is_zcta']=='1'))]
  lines.append('{'+','.join(args)+'},');count+=1
 lines.append('};\n');(ROOT/'local_data.h').write_text('\n'.join(lines))
 print('Offline rows:',count,'ACS ZCTA rows:',len(housing))
if __name__=='__main__':
 p=argparse.ArgumentParser();p.add_argument('--refresh',action='store_true');a=p.parse_args()
 if a.refresh:
  for name,url in URLS.items():
   with urllib.request.urlopen(urllib.request.Request(url,headers={'User-Agent':'NodeCheck/1.5 (postal reference download)'}),timeout=60) as r:data=r.read()
   if name=='housing_full.dat':
    with (ROOT/'housing_2024.csv').open('w',newline='') as f:
     w=csv.writer(f);w.writerow(['zip','occupied','moe'])
     for row in csv.DictReader(io.StringIO(data.decode()),delimiter='|'):
      if row['GEO_ID'].startswith('860Z200US'):w.writerow([row['GEO_ID'][-5:],row['B25003_E001'],row['B25003_M001']])
   else:(ROOT/name).write_bytes(data)
  print('Source metadata may change: review zip_LICENSE.txt and zip_SOURCES.csv before redistribution.')
 build()
