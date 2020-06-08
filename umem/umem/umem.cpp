/*
 * Copyright (c) 2020 Samuel Tulach - All rights reserved
 * Unauthorized copying of this project, via any medium is strictly prohibited
 */

#include <Windows.h>
#include <iostream>
#include <chrono>
#include "Driver.h"

Driver driver;

void Read8Bytes(int times)
{
    uint64_t testVariable = 123;

	for (int i = 0; i < times; i++)
	{
        uint64_t read = driver.Read<uint64_t>(reinterpret_cast<uint64_t>(&testVariable));
        if (read != testVariable)
            printf("[BENCHMARK] Variable does not match!\n");
	}
}

void Read4Bytes(int times)
{
    uint32_t testVariable = 123;

    for (int i = 0; i < times; i++)
    {
        uint32_t read = driver.Read<uint32_t>(reinterpret_cast<uint64_t>(&testVariable));
        if (read != testVariable)
            printf("[BENCHMARK] Variable does not match!\n");
    }
}

void ReadByte(int times)
{
    uint8_t testVariable = 123;

    for (int i = 0; i < times; i++)
    {
        uint8_t read = driver.Read<uint8_t>(reinterpret_cast<uint64_t>(&testVariable));
        if (read != testVariable)
            printf("[BENCHMARK] Variable does not match!\n");
    }
}

int main()
{
    printf("[BENCHMARK] Connecting to driver...\n");
    bool status = driver.Init(GetCurrentProcessId());
    if (!status)
    {
        printf("[BENCHMARK] Failed to init driver\n");
    	return -1;
    }	
    printf("[BENCHMARK] Connected to the driver\n");

    printf("[BENCHMARK] Reading 8 bytes 10000 times...\n");
    auto t1 = std::chrono::high_resolution_clock::now();
    Read8Bytes(10000);
    auto t2 = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();
    printf("[BENCHMARK] Finished in %llu ms\n", duration);

    printf("[BENCHMARK] Reading 4 bytes 10000 times...\n");
    t1 = std::chrono::high_resolution_clock::now();
    Read4Bytes(10000);
    t2 = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();
    printf("[BENCHMARK] Finished in %llu ms\n", duration);

    printf("[BENCHMARK] Reading 1 byte 10000 times...\n");
    t1 = std::chrono::high_resolution_clock::now();
    ReadByte(10000);
    t2 = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();
    printf("[BENCHMARK] Finished in %llu ms\n", duration);

    /*printf("[BENCHMARK] Testing memory leaks...\n");
	while (true)
	{
        printf("[BENCHMARK] Reading 8 bytes 1000000 times...\n");
        auto t1 = std::chrono::high_resolution_clock::now();
        Read8Bytes(1000000);
        auto t2 = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();
        printf("[BENCHMARK] Finished in %llu ms\n", duration);
	}*/
}

