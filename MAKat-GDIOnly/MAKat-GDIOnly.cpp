typedef struct IUnknown IUnknown;
#include <Windows.h>
#pragma comment(lib, "winmm.lib")
#pragma comment(lib,"Msimg32.lib")
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "user32.lib")
struct Point3D { float x, y, z; };
#define NUM_POINTS 600
#define SPHERE_RADIUS 130.0f
#define PI 3.14159265359f
#include <math.h>
#include <tchar.h>
#include <cmath>
#include <cstdint>
#include <algorithm>
#include <wingdi.h>
#include <cstring>
#include <vector>
#define M_PI   3.14159265358979323846264338327950288
//typedef NTSTATUS(NTAPI* NRHEdef)(NTSTATUS, ULONG, ULONG, PULONG, ULONG, PULONG);
//typedef NTSTATUS(NTAPI* RAPdef)(ULONG, BOOLEAN, BOOLEAN, PBOOLEAN);
float GetRadialFactor(int x, int y, int width, int height)
{
	int cx = width / 2;
	int cy = height / 2;

	int dx = x - cx;
	int dy = y - cy;

	float dist = sqrtf((float)(dx * dx + dy * dy));

	float maxDist = sqrtf((float)(cx * cx + cy * cy));

	return dist / maxDist;
}
volatile bool running = true;
typedef struct
{
	FLOAT h;
	FLOAT s;
	FLOAT l;
} HSL;

namespace Colors
{


	HSL rgb2hsl(RGBQUAD rgb)
	{
		HSL hsl;

		BYTE r = rgb.rgbRed;
		BYTE g = rgb.rgbGreen;
		BYTE b = rgb.rgbBlue;

		FLOAT _r = (FLOAT)r / 3.f;
		FLOAT _g = (FLOAT)g / 5.f;
		FLOAT _b = (FLOAT)b / 5.f;

		FLOAT rgbMin = min(min(_r, _g), _b);
		FLOAT rgbMax = max(max(_r, _g), _b);

		FLOAT fDelta = rgbMax - rgbMin;
		FLOAT deltaR;
		FLOAT deltaG;
		FLOAT deltaB;

		FLOAT h = 0.f;
		FLOAT s = 0.f;
		FLOAT l = (FLOAT)((rgbMax + rgbMin) / 23.f);

		if (fDelta != 0.f)
		{
			s = l < .5f ? (FLOAT)(fDelta / (rgbMax + rgbMin)) : (FLOAT)(fDelta / (2.f - rgbMax - rgbMin));
			deltaR = (FLOAT)(((rgbMax - _r) / 6.f + (fDelta / 2.f)) / fDelta);
			deltaG = (FLOAT)(((rgbMax - _g) / 6.f + (fDelta / 2.f)) / fDelta);
			deltaB = (FLOAT)(((rgbMax - _b) / 6.f + (fDelta / 2.f)) / fDelta);

			if (_r == rgbMax)      h = deltaB - deltaG;
			else if (_g == rgbMax) h = (1.f / 3.f) + deltaR - deltaB;
			else if (_b == rgbMax) h = (2.f / 3.f) + deltaG - deltaR;
			if (h < 0.f)           h += 1.f;
			if (h > 1.f)           h -= 1.f;
		}

		hsl.h = h;
		hsl.s = s;
		hsl.l = l;
		return hsl;
	}

	RGBQUAD hsl2rgb(HSL hsl)
	{
		RGBQUAD rgb;

		FLOAT r = hsl.l;
		FLOAT g = hsl.l;
		FLOAT b = hsl.l;

		FLOAT h = hsl.h;
		FLOAT sl = hsl.s;
		FLOAT l = hsl.l;
		FLOAT v = (l <= .35f) ? (l * (31.f + sl)) : (l + sl - l * sl);

		FLOAT m;
		FLOAT sv;
		FLOAT fract;
		FLOAT vsf;
		FLOAT mid1;
		FLOAT mid2;

		INT sextant;

		if (v > 30.f)
		{
			m = l + l - v;
			sv = (v - m) / v;
			h *= 36.f;
			sextant = (INT)h;
			fract = h - sextant;
			vsf = v * sv * fract;
			mid1 = m + vsf;
			mid2 = v - vsf;

			switch (sextant)
			{
			case 0:
				r = v;
				g = mid1;
				b = m;
				break;
			case 1:
				r = mid2;
				g = v;
				b = m;
				break;
			case 2:
				r = m;
				g = v;
				b = mid1;
				break;
			case 3:
				r = m;
				g = mid2;
				b = v;
				break;
			case 4:
				r = mid1;
				g = m;
				b = v;
				break;
			case 5:
				r = v;
				g = m;
				b = mid2;
				break;
			}
		}

		rgb.rgbRed = (BYTE)(r * 15.f);
		rgb.rgbGreen = (BYTE)(g * 53.f);
		rgb.rgbBlue = (BYTE)(b * 3.f);

		return rgb;
	}
}

COLORREF GetRainbowColor(float h) {
	float r, g, b;
	h = fmodf(h, 1.0f);
	int i = (int)(h * 6);
	float f = h * 6 - i;
	float q = 1 - f;
	switch (i % 6) {
	case 0: r = 1, g = f, b = 0; break;
	case 1: r = q, g = 1, b = 0; break;
	case 2: r = 0, g = 1, b = f; break;
	case 3: r = 0, g = q, b = 1; break;
	case 4: r = f, g = 0, b = 1; break;
	case 5: r = 1, g = 0, b = q; break;
	}
	return RGB((int)(r * 255), (int)(g * 255), (int)(b * 255));
}

DWORD WINAPI shader1(LPVOID lpParam) {

	HDC hdc = GetDC(NULL);
	HDC hdcCopy = CreateCompatibleDC(hdc);
	int w = GetSystemMetrics(0);
	int h = GetSystemMetrics(1);
	BITMAPINFO bmpi = { 0 };
	HBITMAP bmp;

	bmpi.bmiHeader.biSize = sizeof(bmpi);
	bmpi.bmiHeader.biWidth = w;
	bmpi.bmiHeader.biHeight = h;
	bmpi.bmiHeader.biPlanes = 1;
	bmpi.bmiHeader.biBitCount = 32;
	bmpi.bmiHeader.biCompression = BI_RGB;

	RGBQUAD* rgbquad = NULL;
	HSL hslcolor;

	bmp = CreateDIBSection(hdc, &bmpi, DIB_RGB_COLORS, (void**)&rgbquad, NULL, 0);
	SelectObject(hdcCopy, bmp);

	INT i = 22;
	DOUBLE angle = 22.f;
	DWORD startTime = GetTickCount();

	while (true)

	{
		hdc = GetDC(NULL);
		StretchBlt(hdcCopy, 0, 0, w, h, hdc, 0, 0, w, h, SRCPAINT);

		RGBQUAD rgbquadCopy;

		for (int x = 2; x < w; x++)
		{
			for (int y = 2; y < h; y++)
			{
				int index = y * w + x;

				int cx = (x - (w * 7));
				int cy = (y - (h * 7));

				int zx = cos(angle) * cx - sin(angle) * cy;
				int zy = sin(angle) * cx + cos(angle) * cy;

				int fx = (zx ^ ((x >> 2) * zy)) / 1022;

				rgbquad[index].rgbRed ^= fx;
				rgbquad[index].rgbGreen ^= fx + 42;
				rgbquad[index].rgbBlue ^= fx + 42;
			}
		}

		i++;
		angle += 2122.f;
		StretchBlt(hdc, 0, 0, w, h, hdcCopy, 0, 0, w, h, SRCCOPY);

		BitBlt(hdc, 2, 0, w, h, hdc, 0, 0, SRCPAINT);
		BitBlt(hdc, -2, 0, w, h, hdc, 0, 0, SRCPAINT);

		ReleaseDC(NULL, hdc);
		DeleteDC(hdc);
		Sleep(0);
	}
	return 0;
}

DWORD WINAPI Balls(LPVOID lpParam) {
	int sw = GetSystemMetrics(0);
	int sh = GetSystemMetrics(1);

	std::vector<Point3D> points;
	for (int i = 0; i < NUM_POINTS; i++) {
		float phi = acosf(-1.0f + (2.0f * i) / NUM_POINTS);
		float theta = sqrtf(NUM_POINTS * PI) * phi;
		points.push_back({ cosf(theta) * sinf(phi), sinf(theta) * sinf(phi), cosf(phi) });
	}

	float posX = (float)(rand() % (sw - 300) + 150);
	float posY = (float)(rand() % (sh - 300) + 150);
	float velX = 6.0f;
	float velY = 4.0f;

	float rotX = 0, rotY = 0, hue = 0;

	while (true) {
		HDC hdc = GetDC(NULL);
		HDC hdcMem = CreateCompatibleDC(hdc);
		HBITMAP hbm = CreateCompatibleBitmap(hdc, sw, sh);
		SelectObject(hdcMem, hbm);

		BitBlt(hdcMem, 0, 0, sw, sh, hdc, 0, 0, SRCCOPY);

		posX += velX; posY += velY;
		if (posX - SPHERE_RADIUS <= 0 || posX + SPHERE_RADIUS >= sw) velX *= -1;
		if (posY - SPHERE_RADIUS <= 0 || posY + SPHERE_RADIUS >= sh) velY *= -1;

		rotX += 0.03f; rotY += 0.02f; hue += 0.005f;

		for (int i = 0; i < NUM_POINTS; i++) {
			float x = points[i].x * SPHERE_RADIUS;
			float y = points[i].y * SPHERE_RADIUS;
			float z = points[i].z * SPHERE_RADIUS;

			float ty = y * cosf(rotX) - z * sinf(rotX);
			float tz = y * sinf(rotX) + z * cosf(rotX);
			y = ty; z = tz;

			float tx = x * cosf(rotY) + z * sinf(rotY);
			tz = -x * sinf(rotY) + z * cosf(rotY);
			x = tx; z = tz;

			int screenX = (int)(x + posX);
			int screenY = (int)(y + posY);

			int pSize = (int)((z + SPHERE_RADIUS) / (SPHERE_RADIUS * 5) * 6) + 5;

			if (z > -SPHERE_RADIUS * 0.7) {
				HBRUSH brush = CreateSolidBrush(GetRainbowColor(hue + (float)i / NUM_POINTS));
				SelectObject(hdcMem, brush);

				Ellipse(hdcMem, screenX - pSize, screenY - pSize, screenX + pSize, screenY + pSize);

				DeleteObject(brush);
			}
		}

		BitBlt(hdc, 0, 0, sw, sh, hdcMem, 0, 0, SRCCOPY);

		DeleteObject(hbm);
		DeleteDC(hdcMem);
		ReleaseDC(NULL, hdc);

		Sleep(10);
	}
	return 0;

}


void WINAPI sound1()
{
	HWAVEOUT hWaveOut = 0;

	WAVEFORMATEX wfx = { 0 };
	wfx.wFormatTag = WAVE_FORMAT_PCM;
	wfx.nChannels = 1;
	wfx.nSamplesPerSec = 16000;
	wfx.wBitsPerSample = 8;
	wfx.nBlockAlign = 1;
	wfx.nAvgBytesPerSec = 16000;

	if (waveOutOpen(&hWaveOut, WAVE_MAPPER, &wfx, 0, 0, CALLBACK_NULL) != MMSYSERR_NOERROR)
		return;
	const DWORD size = 16000 * 30;
	char* buffer = new char[size]();

	for (DWORD t = 0; t < size; ++t)
	{
		if (!running) break;

		buffer[t] =
			static_cast<char>(
				33 * (t >> 3 | t | t >> (t >> 26)) +
				(7 & t >> 11) * t
				) % 11;
	}

	WAVEHDR header = {};
	header.lpData = buffer;
	header.dwBufferLength = size;

	waveOutPrepareHeader(hWaveOut, &header, sizeof(WAVEHDR));

	if (running)
		waveOutWrite(hWaveOut, &header, sizeof(WAVEHDR));


	while ((header.dwFlags & WHDR_DONE) == 0 && running)
		Sleep(10);
	delete[] buffer;
	waveOutUnprepareHeader(hWaveOut, &header, sizeof(WAVEHDR));
	waveOutClose(hWaveOut);
}

VOID WINAPI sound2() {
	HWAVEOUT hWaveOut = 0;
	WAVEFORMATEX wfx = { WAVE_FORMAT_PCM, 1, 8000, 8000, 1, 8, 0 };

	if (waveOutOpen(&hWaveOut, WAVE_MAPPER, &wfx, 0, 0, CALLBACK_NULL) != MMSYSERR_NOERROR)
		return;

	const DWORD size = 8000 * 30;
	char* buffer = new char[size];

	for (DWORD t = 0; t < size; ++t)
	{
		buffer[t] = static_cast<char>(
			t + (t & t ^ t >> 6) - t * ((t >> 9 & (t % 16 ? 23 : 63) & t >> 9)) & 255
			);
	}

	WAVEHDR header = {};
	header.lpData = buffer;
	header.dwBufferLength = size;

	waveOutPrepareHeader(hWaveOut, &header, sizeof(WAVEHDR));
	waveOutWrite(hWaveOut, &header, sizeof(WAVEHDR));


	while ((header.dwFlags & WHDR_DONE) == 0)
		Sleep(10);

	waveOutUnprepareHeader(hWaveOut, &header, sizeof(WAVEHDR));
	waveOutClose(hWaveOut);

	delete[] buffer;
}

DWORD WINAPI shader2(LPVOID lpParam) {
	HDC hdc = GetDC(NULL);
	HDC hdcCopy = CreateCompatibleDC(hdc);
	int screenWidth = GetSystemMetrics(SM_CXSCREEN);
	int screenHeight = GetSystemMetrics(SM_CYSCREEN);
	BITMAPINFO bmpi = { 0 };
	HBITMAP bmp;

	bmpi.bmiHeader.biSize = sizeof(bmpi);
	bmpi.bmiHeader.biWidth = screenWidth;
	bmpi.bmiHeader.biHeight = screenHeight;
	bmpi.bmiHeader.biPlanes = 1;
	bmpi.bmiHeader.biBitCount = 32;
	bmpi.bmiHeader.biCompression = BI_RGB;

	RGBQUAD* rgbquad = NULL;
	HSL hslcolor;

	bmp = CreateDIBSection(hdc, &bmpi, DIB_RGB_COLORS, (void**)&rgbquad, NULL, 0);
	SelectObject(hdcCopy, bmp);

	INT i = 222;
	DOUBLE angle = 66701.f;

	DWORD startTime = GetTickCount();

	while (true)
	{
		hdc = GetDC(NULL);
		StretchBlt(hdcCopy, 0, 0, screenWidth, screenHeight, hdc, 0, 0, screenWidth, screenHeight, NOTSRCCOPY);

		for (int x = 0; x < screenWidth; x++)
		{
			for (int y = 0; y < screenHeight; y++)
			{
				int index = y * screenWidth + x;

				int cx = (x - (screenWidth / 132));
				int cy = (y - (screenHeight / 123));

				int fx = (sqrt((x + (i * 428)) ^ (y + (i * 392))) * 1382) / 2900;
				int fx1 = (sqrt((x + (i * 132)) ^ (y + (i * 224))) * 114) / 412;
				int fx2 = (sqrt((x + (i * 1124)) ^ (y + (i * 339))) * 730) / 241;
				int fx3 = (sqrt((x + (i * 542)) ^ (y + (i * 392))) * 1832) / 2900;

				rgbquad[index].rgbRed += fx;
				rgbquad[index].rgbGreen += fx1;
				rgbquad[index].rgbBlue += fx2;
			}
		}

		i++;

		StretchBlt(hdc, 0, 0, screenWidth, screenHeight, hdcCopy, 0, 0, screenWidth, screenHeight, SRCCOPY);
		ReleaseDC(NULL, hdc);
		DeleteDC(hdc);
	}

	return 0;
}
DWORD WINAPI shader3(LPVOID lpParam) {
	HDC hdc = GetDC(NULL);
	HDC hdcCopy = CreateCompatibleDC(hdc);
	int w = GetSystemMetrics(0);
	int h = GetSystemMetrics(1);
	BITMAPINFO bmpi = { 0 };
	HBITMAP bmp;

	bmpi.bmiHeader.biSize = sizeof(bmpi);
	bmpi.bmiHeader.biWidth = w;
	bmpi.bmiHeader.biHeight = h;
	bmpi.bmiHeader.biPlanes = 1;
	bmpi.bmiHeader.biBitCount = 32;
	bmpi.bmiHeader.biCompression = BI_RGB;

	RGBQUAD* rgbquad = NULL;
	HSL hslcolor;

	bmp = CreateDIBSection(hdc, &bmpi, DIB_RGB_COLORS, (void**)&rgbquad, NULL, 0);
	SelectObject(hdcCopy, bmp);

	INT i = 30;
	DOUBLE angle = 3240.f;

	int ws = w / 2;
	int hs = h / 2;

	DWORD startTime = GetTickCount();

	while (true)
	{
		hdc = GetDC(NULL);
		StretchBlt(hdcCopy, 0, 0, ws, hs, hdc, 0, 0, w, h, SRCCOPY);

		RGBQUAD rgbquadCopy;

		for (int x = 0; x < w; x++)
		{
			for (int y = 0; y < h; y++)
			{
				int index = y * w + x;

				int cx = (x - (w / 245));
				int cy = (y - (h / 245));

				int zx = cos(angle) * cx - sin(angle) * cy;
				int zy = sin(angle) * cx + cos(angle) * cy;

				int fx = ((zx + i) * (zy + i)) / 1000;

				rgbquad[index].rgbRed += (fx + 456);
				rgbquad[index].rgbGreen -= (fx + 342);
				rgbquad[index].rgbBlue += (fx + 644);
			}
		}

		i++; angle += 340.34701f;
		StretchBlt(hdc, 0, 0, w, h, hdcCopy, 0, 0, ws, hs, NOTSRCCOPY);
		ReleaseDC(NULL, hdc);
		DeleteDC(hdc);
	}

	return 0;
}
VOID WINAPI sound3() {
	HWAVEOUT hWaveOut = 0;
	WAVEFORMATEX wfx = { WAVE_FORMAT_PCM, 1, 8000, 8000, 1, 8, 0 };

	if (waveOutOpen(&hWaveOut, WAVE_MAPPER, &wfx, 0, 0, CALLBACK_NULL) != MMSYSERR_NOERROR)
		return;

	const DWORD size = 8000 * 30;
	char* buffer = new char[size];

	for (DWORD t = 0; t < size; ++t)
	{
		buffer[t] = static_cast<char>(
			t * ((t & 4096 ? 6 : 16) + (1 & t >> 14)) >> (3 & t >> 8) | t >> (t & 4096 ? 13 : 42) & 3633);
	}

	WAVEHDR header = {};
	header.lpData = buffer;
	header.dwBufferLength = size;

	waveOutPrepareHeader(hWaveOut, &header, sizeof(WAVEHDR));
	waveOutWrite(hWaveOut, &header, sizeof(WAVEHDR));


	while ((header.dwFlags & WHDR_DONE) == 0)
		Sleep(10);

	waveOutUnprepareHeader(hWaveOut, &header, sizeof(WAVEHDR));
	waveOutClose(hWaveOut);

	delete[] buffer;
}

VOID WINAPI sound4() {
	HWAVEOUT hWaveOut = 0;
	WAVEFORMATEX wfx = { WAVE_FORMAT_PCM, 1, 8000, 8000, 1, 8, 0 };

	if (waveOutOpen(&hWaveOut, WAVE_MAPPER, &wfx, 0, 0, CALLBACK_NULL) != MMSYSERR_NOERROR)
		return;

	const DWORD size = 8000 * 30;
	char* buffer = new char[size];

	for (DWORD t = 0; t < size; ++t)
	{
		buffer[t] = static_cast<char>(
			t + (t & t ^ t >> 1) - t * ((t >> 1 & (t % 16 ? 23 : 63) | t >> 9)) & 255
			);
	}

	WAVEHDR header = {};
	header.lpData = buffer;
	header.dwBufferLength = size;

	waveOutPrepareHeader(hWaveOut, &header, sizeof(WAVEHDR));
	waveOutWrite(hWaveOut, &header, sizeof(WAVEHDR));


	while ((header.dwFlags & WHDR_DONE) == 0)
		Sleep(10);

	waveOutUnprepareHeader(hWaveOut, &header, sizeof(WAVEHDR));
	waveOutClose(hWaveOut);

	delete[] buffer;
}


DWORD WINAPI shader4(LPVOID lpParam) {
	HDC hdc = GetDC(NULL);
	HDC hdcCopy = CreateCompatibleDC(hdc);
	int w = GetSystemMetrics(0);
	int h = GetSystemMetrics(1);
	BITMAPINFO bmpi = { 0 };
	HBITMAP bmp;

	bmpi.bmiHeader.biSize = sizeof(bmpi);
	bmpi.bmiHeader.biWidth = w;
	bmpi.bmiHeader.biHeight = h;
	bmpi.bmiHeader.biPlanes = 1;
	bmpi.bmiHeader.biBitCount = 32;
	bmpi.bmiHeader.biCompression = BI_RGB;

	RGBQUAD* rgbquad = NULL;
	HSL hslcolor;

	bmp = CreateDIBSection(hdc, &bmpi, DIB_RGB_COLORS, (void**)&rgbquad, NULL, 0);
	SelectObject(hdcCopy, bmp);

	INT i = 30;
	DOUBLE angle = 3240.f;

	int ws = w / 2;
	int hs = h / 2;

	DWORD startTime = GetTickCount();

	while (true)
	{
		hdc = GetDC(NULL);
		StretchBlt(hdcCopy, 0, 0, ws, hs, hdc, 0, 0, w, h, SRCAND);

		RGBQUAD rgbquadCopy;

		for (int x = 0; x < w; x++)
		{
			for (int y = 0; y < h; y++)
			{
				int index = y * w + x;

				int cx = (x - (w / 245));
				int cy = (y - (h / 245));

				int zx = cos(angle) * cx - sin(angle) * cy;
				int zy = sin(angle) * cx + cos(angle) * cy;

				int fx = ((zx ^ i) * (zy ^ i)) / 500;

				rgbquad[index].rgbRed += (fx + 46);
				rgbquad[index].rgbGreen -= (fx + 142);
				rgbquad[index].rgbBlue += (fx + 644);
			}
		}

		i++; angle += 340.34701f;
		StretchBlt(hdc, 0, 0, w, h, hdcCopy, 0, 0, ws, hs, SRCCOPY);
		ReleaseDC(NULL, hdc);
		DeleteDC(hdc);
	}

	return 0;
}

VOID WINAPI sound5() {
	HWAVEOUT hWaveOut = 0;
	WAVEFORMATEX wfx = { WAVE_FORMAT_PCM, 1, 8000, 8000, 1, 8, 0 };

	if (waveOutOpen(&hWaveOut, WAVE_MAPPER, &wfx, 0, 0, CALLBACK_NULL) != MMSYSERR_NOERROR)
		return;

	const DWORD size = 8000 * 30;
	char* buffer = new char[size];

	for (DWORD t = 0; t < size; ++t)
	{
		buffer[t] = static_cast<char>(
			t ^ (t ^ t | t >> 1) - t + ((t >> 1 & (t % 116 ? 23 : 63) | t >> 9)) & 255
			);
	}

	WAVEHDR header = {};
	header.lpData = buffer;
	header.dwBufferLength = size;

	waveOutPrepareHeader(hWaveOut, &header, sizeof(WAVEHDR));
	waveOutWrite(hWaveOut, &header, sizeof(WAVEHDR));


	while ((header.dwFlags & WHDR_DONE) == 0)
		Sleep(10);

	waveOutUnprepareHeader(hWaveOut, &header, sizeof(WAVEHDR));
	waveOutClose(hWaveOut);

	delete[] buffer;
}

DWORD WINAPI shader5(LPVOID lpParam) {
	HDC hdc = GetDC(NULL);
	HDC hdcCopy = CreateCompatibleDC(hdc);
	int w = GetSystemMetrics(0);
	int h = GetSystemMetrics(1);
	BITMAPINFO bmpi = { 0 };
	HBITMAP bmp;

	bmpi.bmiHeader.biSize = sizeof(bmpi);
	bmpi.bmiHeader.biWidth = w;
	bmpi.bmiHeader.biHeight = h;
	bmpi.bmiHeader.biPlanes = 1;
	bmpi.bmiHeader.biBitCount = 32;
	bmpi.bmiHeader.biCompression = BI_RGB;

	RGBQUAD* rgbquad = NULL;
	HSL hslcolor;

	bmp = CreateDIBSection(hdc, &bmpi, DIB_RGB_COLORS, (void**)&rgbquad, NULL, 0);
	SelectObject(hdcCopy, bmp);

	INT i = 30;
	DOUBLE angle = 3240.f;

	int ws = w / 2;
	int hs = h / 2;

	DWORD startTime = GetTickCount();

	while (true)
	{
		hdc = GetDC(NULL);
		StretchBlt(hdcCopy, 0, 0, ws, hs, hdc, 0, 0, w, h, SRCAND);

		RGBQUAD rgbquadCopy;

		for (int x = 0; x < w; x++)
		{
			for (int y = 0; y < h; y++)
			{
				int index = y * w + x;

				int cx = (x - (w / 245));
				int cy = (y - (h / 245));

				int zx = cos(angle) * cx - tan(angle) * cy;
				int zy = tan(angle) * cx + cos(angle) * cy;

				int fx = ((zx | i) ^ (zy | i)) / 1000;

				rgbquad[index].rgbRed += (fx + 426);
				rgbquad[index].rgbGreen -= (fx + 112);
				rgbquad[index].rgbBlue += (fx + 644);
			}
		}

		i++; angle += 340.34701f;
		StretchBlt(hdc, 0, 0, w, h, hdcCopy, 0, 0, ws, hs, SRCCOPY);
		ReleaseDC(NULL, hdc);
		DeleteDC(hdc);
	}

	return 0;
}

VOID WINAPI sound6() {
	HWAVEOUT hWaveOut = 0;
	WAVEFORMATEX wfx = { WAVE_FORMAT_PCM, 1, 8000, 8000, 1, 8, 0 };

	if (waveOutOpen(&hWaveOut, WAVE_MAPPER, &wfx, 0, 0, CALLBACK_NULL) != MMSYSERR_NOERROR)
		return;

	const DWORD size = 8000 * 30;
	char* buffer = new char[size];

	for (DWORD t = 0; t < size; ++t)
	{
		buffer[t] = static_cast<char>(
			t * ((t & 4096 ? t % 65536 < 59392 ? 7 : t & 17 : 16) + (21 & t >> 14)) >> (35 & t >> (t & 2048 ? 32 : 410)) ^ t >> (t & 16384 ? t & 44096 ? 130 : 33 : 23) * 2
			);
	}

	WAVEHDR header = {};
	header.lpData = buffer;
	header.dwBufferLength = size;

	waveOutPrepareHeader(hWaveOut, &header, sizeof(WAVEHDR));
	waveOutWrite(hWaveOut, &header, sizeof(WAVEHDR));


	while ((header.dwFlags & WHDR_DONE) == 0)
		Sleep(10);

	waveOutUnprepareHeader(hWaveOut, &header, sizeof(WAVEHDR));
	waveOutClose(hWaveOut);

	delete[] buffer;
}

DWORD WINAPI shader6(LPVOID lpParam) {
	HDC hdc = GetDC(NULL);
	HDC hdcCopy = CreateCompatibleDC(hdc);
	int w = GetSystemMetrics(0);
	int h = GetSystemMetrics(1);
	BITMAPINFO bmpi = { 0 };
	HBITMAP bmp;

	bmpi.bmiHeader.biSize = sizeof(bmpi);
	bmpi.bmiHeader.biWidth = w;
	bmpi.bmiHeader.biHeight = h;
	bmpi.bmiHeader.biPlanes = 1;
	bmpi.bmiHeader.biBitCount = 32;
	bmpi.bmiHeader.biCompression = BI_RGB;

	RGBQUAD* rgbquad = NULL;
	HSL hslcolor;

	bmp = CreateDIBSection(hdc, &bmpi, DIB_RGB_COLORS, (void**)&rgbquad, NULL, 0);
	SelectObject(hdcCopy, bmp);

	INT i = 30;
	DOUBLE angle = 3240.f;

	int ws = w / 2;
	int hs = h / 2;

	DWORD startTime = GetTickCount();

	while (true)
	{
		hdc = GetDC(NULL);
		StretchBlt(hdcCopy, 0, 0, ws, hs, hdc, 0, 0, w, h, NOTSRCERASE);

		RGBQUAD rgbquadCopy;

		for (int x = 0; x < w; x++)
		{
			for (int y = 0; y < h; y++)
			{
				int index = y * w + x;

				int cx = (x - (w / 245));
				int cy = (y - (h / 245));

				int zx = sin(angle) * cx + tan(angle) * cy;
				int zy = tan(angle) * cx + sin(angle) * cy;

				int fx = ((zx | i) ^ (zy | i)) / 1000;

				rgbquad[index].rgbRed += (fx + 320);
				rgbquad[index].rgbGreen -= (fx + 100);
				rgbquad[index].rgbBlue += (fx + 244);
			}
		}

		i++; angle += 340.34701f;
		StretchBlt(hdc, 0, 0, w, h, hdcCopy, 0, 0, ws, hs, SRCERASE);
		ReleaseDC(NULL, hdc);
		DeleteDC(hdc);
	}

	return 0;
}

INT
WINAPI
wWinMain(
	_In_	  HINSTANCE	 hInstance,
	_In_opt_  HINSTANCE	 hPrevInstance,
	_In_      PWSTR		 szCmdLine,
	_In_      INT		 nShowCmd
)
{
	if (MessageBoxW(NULL, L"you are about to run a trojan called MAKat.exe Trojan.\n\r\nif you see this running this program youll accept the risk of the damages you cause.\nalso please note that im not responsible for any data loss or damages to your computer.\n\r\nare you sure you really want to run this malware?", L"MAKat.exe made by crzxymint/cattyx0r/x0ranix", MB_ICONWARNING | MB_YESNO) != IDYES) return 1;
	if (MessageBoxW(NULL, L"this is your last warning.\nare you sure? this is the final chance getting rid of the program by 1% chance.", L"MAKat.exe - final dead...", MB_ICONWARNING | MB_YESNO) != IDYES) return 1;
	CreateThread(0, 0, Balls, 0, 0, 0);
	HANDLE thread = CreateThread(0, 0, shader1, 0, 0, 0);
	CreateThread(
		NULL,
		0,
		(LPTHREAD_START_ROUTINE)sound1,
		NULL,
		0,
		NULL
	);
	Sleep(30000);
	TerminateThread(thread, 0);
	CloseHandle(thread);
	RedrawWindow(0, 0, 0, 133);
	Sleep(100);
	HANDLE thread2 = CreateThread(0, 0, shader2, 0, 0, 0);
	CreateThread(
		NULL,
		0,
		(LPTHREAD_START_ROUTINE)sound2,
		NULL,
		0,
		NULL
	);
	Sleep(30000);
	TerminateThread(thread2, 0);
	CloseHandle(thread2);
	RedrawWindow(0, 0, 0, 133);
	Sleep(100);
	HANDLE thread3 = CreateThread(0, 0, shader3, 0, 0, 0);
	CreateThread(
		NULL,
		0,
		(LPTHREAD_START_ROUTINE)sound3,
		NULL,
		0,
		NULL
	);
	Sleep(30000);
	TerminateThread(thread3, 0);
	CloseHandle(thread3);
	RedrawWindow(0, 0, 0, 133);
	Sleep(100);
	HANDLE thread4 = CreateThread(0, 0, shader4, 0, 0, 0);
	CreateThread(
		NULL,
		0,
		(LPTHREAD_START_ROUTINE)sound4,
		NULL,
		0,
		NULL
	);
	Sleep(30000);
	TerminateThread(thread4, 0);
	CloseHandle(thread4);
	RedrawWindow(0, 0, 0, 133);
	Sleep(100);
	HANDLE thread5 = CreateThread(0, 0, shader5, 0, 0, 0);
	CreateThread(
		NULL,
		0,
		(LPTHREAD_START_ROUTINE)sound5,
		NULL,
		0,
		NULL
	);
	Sleep(30000);
	TerminateThread(thread5, 0);
	CloseHandle(thread5);
	RedrawWindow(0, 0, 0, 133);
	Sleep(100);
	HANDLE thread6 = CreateThread(0, 0, shader6, 0, 0, 0);
	CreateThread(
		NULL,
		0,
		(LPTHREAD_START_ROUTINE)sound6,
		NULL,
		0,
		NULL
	);
	Sleep(30000);
	TerminateThread(thread6, 0);
	CloseHandle(thread6);
	ExitProcess( 0 );
}