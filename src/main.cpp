#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <cstdint>
#include <string>

constexpr std::uintptr_t kContraptionRVA = 0x1267498;
constexpr std::uintptr_t kLoadingScreenVftRVA = 0xF838E8;
constexpr std::uintptr_t kLoadingScreenOffset = 0x1F8;
constexpr std::uintptr_t kImageBoxOffset = 0x30;

constexpr const char *kImageTextFuncName = "?setImageTexture@ImageBox@MyGUI@@QEAAXAEBV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@@Z";

constexpr const char *kCustomImagePath = "gui_loadingscreen_axolot.png";

using MyGUISetImageTextureFunc = void(__thiscall *)(void *, const std::string &);
MyGUISetImageTextureFunc pMyGUISetImageTexture = nullptr;

using SetNewLoadingMenuImageFunc = std::int64_t(__fastcall *)(std::int64_t, std::uint64_t, char);
SetNewLoadingMenuImageFunc pSetNewLoadingMenuImage = nullptr;

void OverrideLoadingScreenImage()
{
    // Get to the image box of the loading screen and set our image as the texture

    const auto pBase = reinterpret_cast<std::uintptr_t>(GetModuleHandleW(nullptr));
    const auto pContraption = *reinterpret_cast<std::uintptr_t *>(pBase + kContraptionRVA);
    const auto pLoadingScreen = *reinterpret_cast<std::uintptr_t *>(pContraption + kLoadingScreenOffset);
    const auto pImageBox = *reinterpret_cast<std::uintptr_t *>(pLoadingScreen + kImageBoxOffset);
    pMyGUISetImageTexture(reinterpret_cast<void *>(pImageBox), kCustomImagePath);
}

std::int64_t __fastcall hkSetNewLoadingMenuImage(std::int64_t pLoadingScreen, std::uint64_t a2, char a3)
{
    auto res = pSetNewLoadingMenuImage(pLoadingScreen, a2, a3);

    OverrideLoadingScreenImage();

    return res;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    if (ul_reason_for_call != DLL_PROCESS_ATTACH)
    {
        return TRUE;
    }

    // Load functions from MyGUI

    const auto hMyGUI = GetModuleHandleW(L"MyGUIEngine.dll");
    pMyGUISetImageTexture = reinterpret_cast<MyGUISetImageTextureFunc>(GetProcAddress(hMyGUI, kImageTextFuncName));

    if (!pMyGUISetImageTexture)
    {
        MessageBoxA(nullptr, "Failed to get MyGUI::ImageBox::setImageTexture function address", "Error", MB_ICONERROR);
        return FALSE;
    }

    // Hook the function responsible for setting the loading screen image to override it with our own image

    const auto pBase = reinterpret_cast<std::uintptr_t>(GetModuleHandleW(nullptr));
    const auto pLoadingScreenVft = reinterpret_cast<void **>(pBase + kLoadingScreenVftRVA);

    pSetNewLoadingMenuImage = reinterpret_cast<SetNewLoadingMenuImageFunc>(pLoadingScreenVft[1]);

    DWORD oldProtect;
    VirtualProtect(&pLoadingScreenVft[1], sizeof(std::uintptr_t), PAGE_EXECUTE_READWRITE, &oldProtect);
    *(&pLoadingScreenVft[1]) = &hkSetNewLoadingMenuImage;
    VirtualProtect(&pLoadingScreenVft[1], sizeof(std::uintptr_t), oldProtect, &oldProtect);
    FlushInstructionCache(GetCurrentProcess(), nullptr, 0);

    // Pin our module (increase ref count) so that the hook code stays loaded

    HMODULE self{};
    GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, reinterpret_cast<LPCWSTR>(&hkSetNewLoadingMenuImage), &self);

    return TRUE;
}