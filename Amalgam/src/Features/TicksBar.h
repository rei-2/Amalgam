#pragma once
#include "../../Module.h"   // <-- adjust this include to match your project's Module base path

class TicksBar : public Module {
public:
    TicksBar();

    // Called every frame from your HUD/render manager
    void OnRender() override;

    // Optional: hook this into your tick loop if you want the bar to drain/fill
    void SetProgress(float p);
    void SetReady(bool ready);

private:
    float progress = 1.0f;   // 0.0 = empty, 1.0 = full
    bool  ready    = true;
};
