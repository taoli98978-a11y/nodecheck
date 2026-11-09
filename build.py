import subprocess as sp,pathlib
import os
os.chdir(pathlib.Path(__file__).resolve().parent)
root=pathlib.Path('.')
flags='-std=c11 -Os -mabi=ms -maccumulate-outgoing-args -fshort-wchar -fpie -mcmodel=small -fno-stack-protector -fno-asynchronous-unwind-tables -fno-unwind-tables -ffreestanding -fno-builtin -fcf-protection=none -fno-ident -fstack-clash-protection'.split()
sp.run(['gcc',*flags,'-c','main.c','-o','main.o'],check=True)
groups={'kernel32':'GetLastError GetProcessHeap HeapAlloc HeapFree MultiByteToWideChar WideCharToMultiByte ExitProcess GetModuleHandleW CreateThread CloseHandle lstrlenW GlobalAlloc GlobalLock GlobalFree GlobalUnlock InterlockedIncrement InterlockedDecrement','msvcrt':'memcpy memset strlen strcmp strncmp _stricmp atoi strstr strchr strrchr atof _snprintf sin cos sqrt atan2','ws2_32':'WSAStartup getaddrinfo freeaddrinfo socket connect closesocket recv send setsockopt inet_pton','secur32':'AcquireCredentialsHandleW InitializeSecurityContextW FreeCredentialsHandle DeleteSecurityContext FreeContextBuffer QueryContextAttributesW EncryptMessage DecryptMessage','winhttp':'WinHttpOpen WinHttpConnect WinHttpOpenRequest WinHttpSetTimeouts WinHttpSendRequest WinHttpReceiveResponse WinHttpQueryHeaders WinHttpReadData WinHttpCloseHandle','gdi32':'GetStockObject CreateFontW SetTextColor SetBkColor','user32':'LoadIconW MessageBoxW RegisterClassW CreateWindowExW LoadCursorW SetProcessDPIAware ShowWindow UpdateWindow GetMessageW TranslateMessage IsDialogMessageW GetClientRect MoveWindow SetWindowTextW GetWindowTextW EnableWindow PostMessageW SetWindowPos DestroyWindow OpenClipboard CloseClipboard EmptyClipboard SetFocus SetClipboardData DispatchMessageW DefWindowProcW SendMessageW PostQuitMessage GetSysColorBrush InvalidateRect'}
# Interlocked operations are compiled locally to avoid relying on optional export aliases.
for lib,syms in groups.items():
 path=root/(lib+'.def');path.write_text('LIBRARY '+lib+'.dll\nEXPORTS\n'+'\n'.join(syms.split()))
 sp.run(['ld','-mi386pep','--dll',str(path),'--out-implib','lib'+lib+'.a','-o','unused.dll'],capture_output=True)
sp.run(['ld','-mi386pep','--subsystem','windows:6.1','--image-base','0x400000','--disable-high-entropy-va','-e','entry','-s','-o','ProxyCheck-Beta.exe','main.o',*['lib'+x+'.a' for x in groups]],check=True)
