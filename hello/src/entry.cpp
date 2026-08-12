// gw2-nexus — walking-skeleton addon.
//
// The thinnest possible Nexus addon: it adopts Nexus's shared ImGui context and
// draws a single window. Its whole job is to prove the build-and-run loop
// (CMake -> CI -> Nexus -> in-game render). See docs/specs/002-build-skeleton/.

#include <Windows.h>

#include "imgui.h"
#include "Nexus.h"

namespace {

AddonDefinition_t g_AddonDef{};
AddonAPI_t*       g_API = nullptr;
bool              g_WindowOpen = true;

// Registered as an RT_Render callback; Nexus calls it every frame.
void AddonRender()
{
    if (!g_WindowOpen) { return; }

    // Center the window on first appearance so it can't be missed; the user can
    // move it afterwards. (GetMainViewport() doesn't exist in ImGui 1.80, so
    // center via the display size.)
    const ImVec2 display = ImGui::GetIO().DisplaySize;
    ImGui::SetNextWindowPos(ImVec2(display.x * 0.5f, display.y * 0.5f),
                            ImGuiCond_FirstUseEver, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(340.0f, 130.0f), ImGuiCond_FirstUseEver);

    if (ImGui::Begin("gw2-nexus \xE2\x80\x94 hello", &g_WindowOpen))
    {
        ImGui::Text("Hello from gw2-nexus.");
        ImGui::Text("Walking skeleton: build -> CI -> Nexus -> render.");
    }
    ImGui::End();
}

// Nexus calls this once when the addon loads, handing us the API function table.
void AddonLoad(AddonAPI_t* aApi)
{
    g_API = aApi;

    // A DLL has its own ImGui globals/heap — adopt Nexus's context + allocators
    // so our draw calls land on the same context the loader renders.
    ImGui::SetCurrentContext(static_cast<ImGuiContext*>(aApi->ImguiContext));
    // Raw signatures (not the ImGuiMemAllocFunc typedefs, which don't exist in
    // ImGui 1.80 — the version the current Nexus host runs).
    ImGui::SetAllocatorFunctions(
        reinterpret_cast<void* (*)(size_t, void*)>(aApi->ImguiMalloc),
        reinterpret_cast<void  (*)(void*, void*)>(aApi->ImguiFree));

    aApi->GUI_Register(RT_Render, AddonRender);

    if (aApi->Log) { aApi->Log(LOGL_INFO, "gw2-nexus", "hello addon loaded"); }

    // Nexus draws this toast with its OWN ImGui, so it appears even if our
    // window rendering is still off — a guaranteed "it's alive" signal.
    if (aApi->GUI_SendAlert) { aApi->GUI_SendAlert("gw2-nexus: hello addon loaded"); }
}

// Nexus calls this on unload/reload; free everything we registered.
void AddonUnload()
{
    if (g_API && g_API->GUI_Deregister) { g_API->GUI_Deregister(AddonRender); }
    g_API = nullptr;
}

} // namespace

// Nexus looks up this exported symbol to discover the addon.
extern "C" __declspec(dllexport) AddonDefinition_t* GetAddonDef()
{
    g_AddonDef.Signature   = 0x67573031; // provisional unique id; finalize per addon
    g_AddonDef.APIVersion  = NEXUS_API_VERSION;
    g_AddonDef.Name        = "gw2-nexus hello";
    g_AddonDef.Version     = AddonVersion_t{ 0, 1, 0, 0 };
    g_AddonDef.Author      = "Kyarha";
    g_AddonDef.Description = "Walking-skeleton addon: proves the build -> CI -> render loop.";
    g_AddonDef.Load        = AddonLoad;
    g_AddonDef.Unload      = AddonUnload;
    g_AddonDef.Flags       = AF_None;
    // Umbrella skeleton: no auto-update. Per-addon UP_GitHub is wired when an
    // addon is extracted to its own repo (ADR-0001).
    g_AddonDef.Provider    = UP_None;
    g_AddonDef.UpdateLink  = nullptr;
    return &g_AddonDef;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ulReasonForCall, LPVOID /*lpReserved*/)
{
    switch (ulReasonForCall)
    {
        case DLL_PROCESS_ATTACH: DisableThreadLibraryCalls(hModule); break;
        case DLL_PROCESS_DETACH: break;
    }
    return TRUE;
}
