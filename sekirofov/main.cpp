#include <iostream>
#include <cmath>
#include <Windows.h>

#undef max

template<typename T>
T read_mem(const HANDLE proc, const uintptr_t address)
{
	T out;
	if (!ReadProcessMemory(proc, (void*)(address), (void*)(&out), sizeof(out), nullptr))
	{
		std::cerr << "ReadProcessMemory failed " << GetLastError() << " " << std::endl;
		return (T)(0);
	}

	return out;
}

template<typename T>
void write_mem(const HANDLE proc, const uintptr_t address, const T value)
{
	if (!WriteProcessMemory(proc, (void*)(address), (void*)(&value), sizeof(value), nullptr))
		std::cerr << "WriteProcessMemory failed " << GetLastError() << std::endl;
}

bool modify_camera(const HANDLE proc, const float fov)
{
	const auto SoloParamRepository = read_mem<uintptr_t>(proc, 0x143B84880);
	if (SoloParamRepository == 0)
		return false;

	const auto deref1 = read_mem<uintptr_t>(proc, SoloParamRepository + 0x930);
	if (deref1 == 0)
		return false;

	const auto deref2 = read_mem<uintptr_t>(proc, deref1 + 0x70);
	if (deref2 == 0)
		return false;

	const auto deref3 = read_mem<uintptr_t>(proc, deref2 + 0x70);
	if (deref3 == 0)
		return false;

	const auto deg2rad = 3.1415926F / 180.F;

	const auto first_base = deref3 + 0xE50;
	const auto prev_fov = read_mem<float>(proc, first_base + 0x14);
	const auto prev_mult = tanf(prev_fov / 2.F * deg2rad) / tanf(43.F / 2.F * deg2rad);
	const auto mult = tanf(fov / 2.F * deg2rad) / tanf(43.F / 2.F * deg2rad);

	for (auto i = 0; i < 141; i++)
	{
		const auto base = first_base + i * 0x64;
		const auto dist = read_mem<float>(proc, base);
		const auto fov = read_mem<float>(proc, base + 0x14);

		write_mem<float>(proc, base, dist / mult * prev_mult);
		write_mem<float>(proc, base + 0x14, atanf(tanf(fov * deg2rad / 2) * mult / prev_mult) * 2 / deg2rad);
	}

	return true;
}

bool find_process(const float fov)
{
	const auto wnd = FindWindow(nullptr, "Sekiro");
	if (wnd == nullptr)
		return false;

	DWORD pid;
	GetWindowThreadProcessId(wnd, &pid);

	const auto proc = OpenProcess(PROCESS_VM_OPERATION | PROCESS_VM_READ | PROCESS_VM_WRITE, FALSE, pid);
	if (proc == nullptr)
	{
		std::cerr << "Found window but couldn't obtain process handle. Try running as administrator." << std::endl;
		return true;
	}

	while (!modify_camera(proc, fov))
		Sleep(1);

	CloseHandle(proc);
	CloseHandle(wnd);
	return true;
}

int main(const int argc, const char *argv[])
{
	float fov;

	if (argc > 1)
	{
		char *endptr;
		fov = strtof(argv[1], &endptr);
		if (endptr == nullptr || fov <= 0.F || fov >= 180.F)
		{
			std::cerr << "Invalid argument." << std::endl;
			return 1;
		}
	}
	else
	{
		std::cout << "Input FOV (vertical)" << std::endl;
		std::cout << "If you use 90 in Source Engine games, you want 74." << std::endl;
		std::cout << "The default is 43." << std::endl;
		std::cout << "You can also specify FOV as a command line option." << std::endl;
		while (true)
		{
			std::cin >> fov;
			if (std::cin.good() && fov < 180.F && fov > 0.F)
				break;

			std::cout << "Invalid value." << std::endl;
			std::cin.clear();
			std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
		}
	}

	while (!find_process(fov))
		Sleep(1);

	return 0;
}