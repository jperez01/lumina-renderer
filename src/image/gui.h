//
// Created by juperez on 5/26/23.
//

#pragma once

#include "core/common.h"
#include "block.h"
#include <nanogui/screen.h>

LUMINA_NAMESPACE_BEGIN

class LuminaScreen : public nanogui::Screen {
public:
    LuminaScreen(const ImageBlock& block);
    void draw_contents() override;
private:
    const ImageBlock &m_block;
    nanogui::ref<nanogui::Shader> m_shader;
    nanogui::ref<nanogui::Texture> m_texture;
    nanogui::ref<nanogui::RenderPass> m_renderPass;
    float m_scale = 1.0f;
};
LUMINA_NAMESPACE_END
