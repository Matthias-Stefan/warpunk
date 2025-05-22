#pragma once

#include "warpunk.core/src/defines.h"
#include "warpunk.core/src/renderer/renderer_backend.h"

namespace vulkan_renderer 
{
    //* */
    b8 renderer_startup(renderer_config_s renderer_config);

    /** */
    b8 renderer_shutdown();

    /** */
    void renderer_begin_frame();

    /** */
    void renderer_end_frame();

    /** */
    buffer_handle_s renderer_create_buffer(s32 size, void* data);

    /** */
    void renderer_destroy_buffer(buffer_handle_s buffer_handle);

    /** */
    texture_handle_s renderer_create_texture(s32 width, s32 height, void* data);

    /** */
    void renderer_destroy_texture(texture_handle_s texture_handle);

    /** */
    void renderer_draw(void* vertex_array, void* material);


    /** */
    void renderer_device_query_swapchain_support(struct _vulkan_swapchain_support_info_s* out_swapchain_support);
}
/**
# Vulkan Renderlayer – Roadmap

## ✅ 1. Vulkan Instance Setup
- [x] Application Name & Engine Name setzen
- [x] Vulkan-Instance mit `vkCreateInstance` erzeugen
- [x] Ergebnis prüfen mit `vulkan_eval_result`

## 🔜 2. Validation Layers (Debug-Modus)
- [x] Check: Debug-Build aktiv?
- [x] Validation Layers aktivieren (`VK_LAYER_KHRONOS_validation`)
- [ ] Optional: Debug Messenger einbauen (für Runtime-Errors & Warnings)

## 🔜 3. Extensions (z. B. für Window-Systeme)
- [x] Benötigte Extensions abfragen (z. B. mit GLFW)
- [x] `VK_KHR_surface` + Plattform-spezifisch (Win32/XCB/etc.) aktivieren
- [x] In `VkInstanceCreateInfo` korrekt eintragen

## 🔜 4. Surface Creation
- [x] Falls Window vorhanden: `VkSurfaceKHR` erzeugen
- [x] Platform-spezifisch oder mit GLFW/SDL etc.

## 🔜 5. Physical Device Auswahl (GPU)
- [x] Alle verfügbaren GPUs abfragen
- [x] Check: Unterstützt gewünschte Queue-Familien
- [x] Check: Unterstützt Swapchain, Features, Extensions
- [x] Auswahl-Logik: Bevorzuge diskrete GPU

## 🔜 6. Logical Device & Queues
- [x] Queue-Familien wählen (Graphics, Present, evtl. Compute)
- [x] `VkDevice` erzeugen mit gewählten Queues
- [x] Handles zu Queues holen (`vkGetDeviceQueue`)

## 🔜 7. Swapchain Setup
- [ ] Surface Capabilities abfragen
- [ ] Swapchain-Format und Present-Modus wählen
- [ ] `VkSwapchainKHR` erstellen
- [ ] Image Views anlegen

## 🔜 8. Render Pass & Pipeline
- [ ] Render Pass anlegen
- [ ] Framebuffer konfigurieren
- [ ] Shader laden (SPIR-V)
- [ ] Graphics Pipeline erstellen

## 🔜 9. Drawing & Presentation
- [ ] Command Buffers erstellen und aufzeichnen
- [ ] Synchronisation mit Semaphores & Fences
- [ ] Image aus Swapchain holen → zeichnen → präsentieren

## 🔜 10. Extras & Features
- [ ] ImGui integrieren
- [ ] Post-Processing Pass
- [ ] Kamera & Transform-System
- [ ] Lighting & Shadow Mapping
- [ ] Fog, Haze, Depth of Field

 
*/
