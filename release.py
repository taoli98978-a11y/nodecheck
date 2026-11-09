from PIL import Image,ImageDraw
from pathlib import Path
from struct import pack,pack_into,unpack_from
import io,zipfile,os
os.chdir(Path(__file__).resolve().parent)
# Draw at a large resolution, then produce separate Windows icon sizes.
im=Image.new('RGBA',(512,512));d=ImageDraw.Draw(im)
mask=Image.new('L',im.size);ImageDraw.Draw(mask).rounded_rectangle((12,12,500,500),105,fill=255)
for y in range(512):
 t=y/511;d.line((0,y,511,y),fill=(int(45-28*t),int(104-61*t),int(184-76*t),255))
im.putalpha(mask)
d.ellipse((115,76,397,356),fill='white');d.polygon([(133,282),(379,282),(267,438),(255,447),(244,438)],fill='white')
d.ellipse((160,119,352,311),fill=(9,111,145));points=[(199,215),(237,252),(310,173)];d.line(points,fill=(108,255,205),width=22,joint='curve')
for x,y in points:d.ellipse((x-10,y-10,x+10,y+10),fill=(108,255,205))
def aligned(x,a):return (x+a-1)//a*a
def pad(d):return d+bytes((-len(d))%4)
def u(s):return (s+'\0').encode('utf-16le')
def block(key,value=b'',children=(),string=False):
 b=pad(pack('<HHH',0,len(value)//(2 if string else 1),int(string))+u(key))+value
 for c in children:b=pad(b)+c
 return pack('<H',len(b))+b[2:]
strings={'FileDescription':'节点核查简易版','ProductName':'节点核查简易版','FileVersion':'1.5.2','ProductVersion':'1.5.2','OriginalFilename':'节点核查简易版.exe'}
fixed=pack('<13I',0xfeef04bd,0x10000,0x10005,0x20000,0x10005,0x20000,63,0,0x40004,1,0,0,0)
v=block('VS_VERSION_INFO',fixed,[block('StringFileInfo',children=[block('080404B0',children=[block(k,u(val),string=True) for k,val in strings.items()],string=True)],string=True),block('VarFileInfo',children=[block('Translation',pack('<HH',2052,1200))],string=True)])
icons={};entries=b''
for i,n in enumerate([16,20,24,32,40,48,64,128,256],1):
 out=io.BytesIO();im.resize((n,n),Image.Resampling.LANCZOS).save(out,format='PNG');data=out.getvalue();icons[i]={2052:data};entries+=pack('<BBBBHHIH',n%256,n%256,0,0,1,32,len(data),i)
tree={3:icons,14:{101:{2052:pack('<HHH',0,1,len(icons))+entries}},16:{1:{2052:v}}}
b=bytearray(Path('ProxyCheck-Beta.exe').read_bytes());pe=unpack_from('<I',b,60)[0];opt=pe+24;count=unpack_from('<H',b,pe+6)[0];rva=unpack_from('<I',b,opt+56)[0];res=bytearray()
def reserve(n):i=len(res);res.extend(bytes(n));return i
def directory(t):
 i=reserve(16+8*len(t));pack_into('<H',res,i+14,len(t))
 for j,(key,item) in enumerate(sorted(t.items())):
  if isinstance(item,dict):off=directory(item)|0x80000000
  else:
   off=reserve(16);pos=reserve(aligned(len(item),4));res[pos:pos+len(item)]=item;pack_into('<IIII',res,off,rva+pos,len(item),0,0)
  pack_into('<II',res,i+16+j*8,key,off)
 return i
directory(tree);raw=aligned(len(b),512);size=aligned(len(res),512);sec=opt+unpack_from('<H',b,pe+20)[0]+count*40;assert sec+40<=unpack_from('<I',b,opt+60)[0]
pack_into('<8sIIIIIIHHI',b,sec,b'.rsrc',len(res),rva,size,raw,0,0,0,0,0x40000040);pack_into('<H',b,pe+6,count+1);pack_into('<II',b,opt+128,rva,len(res));pack_into('<I',b,opt+56,aligned(rva+len(res),4096));pack_into('<I',b,opt+8,unpack_from('<I',b,opt+8)[0]+size);pack_into('<I',b,opt+64,0);b.extend(bytes(raw-len(b))+res+bytes(size-len(res)))
p=Path('节点核查简易版.exe');p.write_bytes(b)
with zipfile.ZipFile('../NodeCheck_1.5.2_Win64.zip','w',zipfile.ZIP_DEFLATED) as z:
 for name in [p.name,'使用说明.txt','THIRD_PARTY.txt','测试记录.txt']:z.write(name,name)
with zipfile.ZipFile('../NodeCheck_1.5.2_Source.zip','w',zipfile.ZIP_DEFLATED) as z:
 for name in ['main.c','win.h','net.h','geo.h','http.h','lookup.h','places.h','postal_data.h','build.py','release.py','update_postal_data.py','fixture.c','test150.c','test151.c','test152.c','test_http.c','build_reference.py','local_data.h','zip_reference.csv','housing_2024.csv','zip_LICENSE.txt','zip_SOURCES.csv','core_cli.c','test_fetch.py','README.md','使用说明.txt','THIRD_PARTY.txt','测试记录.txt']:z.write(name,'NodeCheck_1.5.2_Source/'+name)
for name in ['../NodeCheck_1.5.2_Win64.zip','../NodeCheck_1.5.2_Source.zip']:
 with zipfile.ZipFile(name) as z:assert z.testzip() is None
print('Packaged EXE and complete source archives.')
