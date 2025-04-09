#pragma once

#include "warpunk.core/defines.h"
#include "warpunk.core/renderer/renderer_backend.h"

namespace vulkan_renderer 
{
    //* */
    b8 renderer_startup(renderer_config_t renderer_config);

    /** */
    b8 renderer_shutdown();

    /** */
    void renderer_begin_frame();

    /** */
    void renderer_end_frame();

    /** */
    buffer_handle_t renderer_create_buffer(s32 size, void* data);

    /** */
    void renderer_destroy_buffer(buffer_handle_t buffer_handle);

    /** */
    texture_handle_t renderer_create_texture(s32 width, s32 height, void* data);

    /** */
    void renderer_destroy_texture(texture_handle_t texture_handle);

    /** */
    void renderer_draw(void* vertex_array, void* material);
}
/**
# Vulkan Renderlayer – Roadmap

## ✅ 1. Vulkan Instance Setup
- [x] Application Name & Engine Name setzen
- [x] Vulkan-Instance mit `vkCreateInstance` erzeugen
- [x] Ergebnis prüfen mit `vulkan_eval_result`

## 🔜 2. Validation Layers (Debug-Modus)
- [ ] Check: Debug-Build aktiv?
- [ ] Validation Layers aktivieren (`VK_LAYER_KHRONOS_validation`)
- [ ] Optional: Debug Messenger einbauen (für Runtime-Errors & Warnings)

## 🔜 3. Extensions (z. B. für Window-Systeme)
- [ ] Benötigte Extensions abfragen (z. B. mit GLFW)
- [ ] `VK_KHR_surface` + Plattform-spezifisch (Win32/XCB/etc.) aktivieren
- [ ] In `VkInstanceCreateInfo` korrekt eintragen

## 🔜 4. Surface Creation
- [ ] Falls Window vorhanden: `VkSurfaceKHR` erzeugen
- [ ] Platform-spezifisch oder mit GLFW/SDL etc.

## 🔜 5. Physical Device Auswahl (GPU)
- [ ] Alle verfügbaren GPUs abfragen
- [ ] Check: Unterstützt gewünschte Queue-Familien
- [ ] Check: Unterstützt Swapchain, Features, Extensions
- [ ] Auswahl-Logik: Bevorzuge diskrete GPU

## 🔜 6. Logical Device & Queues
- [ ] Queue-Familien wählen (Graphics, Present, evtl. Compute)
- [ ] `VkDevice` erzeugen mit gewählten Queues
- [ ] Handles zu Queues holen (`vkGetDeviceQueue`)

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
