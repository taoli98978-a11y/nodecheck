# NodeCheck · 节点核查简易版

Windows 10/11 x64 原生代理连接与地区参考查询工具。当前源码版本 1.5.2。

## 功能
- 支持 HTTP、HTTPS、SOCKS5、SOCKS5H；支持 `host:port:user:password` 和 `socks5h://user:password@host:port`。
- 通过用户提供的代理查询出口 IP；用 ipwho.is 和 IP2Location.io 对照同一个 IP。
- 展示州、城市、经纬度，提供美国住宅邮区与电话区号参考。
- 复制节点、中文地区、参考邮编和电话区号；不同城市的两组结果分别复制。
- 可调窗口、后台查询，无登录系统或 Gemini 密钥要求。

## 使用与限制
发布源码对应的历史构建和测试说明见 `使用说明.txt`、`测试记录.txt`。
IP 数据库位置不是设备真实位置。ACS 住宅统计及邮区代表点不是具体住宅地址。
本地数据不是实时更新；在线查询可能限流、失败或存在差异。
程序通过单个测试站点成功连接，不保证所有应用、图片和视频均可访问。
本仓库当前不提供受信任代码签名，不建议为运行测试程序关闭系统安全保护。
已做 Linux 核心及网络模拟测试，尚未完成 Windows 实机验证。

## Build and tests
Build environment: Linux x86-64, GCC, GNU binutils with `i386pep` target, Python 3 and Pillow.
The build uses the Microsoft x64 ABI and links a native PE GUI executable against Windows system DLLs.

```sh
python3 -m pip install Pillow
for test in test151 test152 test_http; do
  gcc -w -O1 -ffunction-sections -fdata-sections -Wl,--gc-sections "$test.c" -lm -o "$test"
  ./"$test"
done
python3 build.py
python3 release.py
```

The executable and ZIP artifacts are generated locally. Never commit credentials, private signing keys or test caches.
The source release packer describes the original 1.5.2 source set; this repository also adds publication documentation.

## Licenses and data
Original program code: MIT (`LICENSE`). Bundled data retains the attribution and licenses in `THIRD_PARTY.txt`, `zip_LICENSE.txt`, `zip_SOURCES.csv`.
ZIP code data from [zipcodes.pro](https://zipcodes.pro/download), CC BY 4.0; GeoNames, CC BY 4.0; U.S. Census Bureau ACS government data.
IP Geolocation by [IP2Location.io](https://www.ip2location.io/) and [ipwho.is](https://ipwho.is/).
Map data: OpenStreetMap contributors (ODbL).

## Privacy policy
See [PRIVACY.md](PRIVACY.md) for network requests and handling of proxy credentials.

## Code signing policy
See [CODE_SIGNING.md](CODE_SIGNING.md). SignPath approval has not been obtained; no affiliation or endorsement is claimed.

## Support
Report reproducible problems via GitHub Issues. Remove all proxy usernames, passwords, access tokens and personal data before posting.
