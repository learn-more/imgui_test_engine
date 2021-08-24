// dear imgui
// (tests: widgets)

#define _CRT_SECURE_NO_WARNINGS
#include <limits.h>
#include "imgui.h"
#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui_internal.h"
#include "imgui_tests.h"
#include "shared/imgui_capture_tool.h"
#include "shared/imgui_utils.h"
#include "test_engine/imgui_te_engine.h"      // IM_REGISTER_TEST()
#include "test_engine/imgui_te_context.h"
#include "libs/Str/Str.h"

// Warnings
#ifdef _MSC_VER
#pragma warning (disable: 4100) // unreferenced formal parameter
#pragma warning (disable: 4127) // conditional expression is constant
#else
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#endif

// Helpers
static inline bool operator==(const ImVec2& lhs, const ImVec2& rhs)     { return lhs.x == rhs.x && lhs.y == rhs.y; }    // for IM_CHECK_EQ()

typedef ImGuiDataTypeTempStorage ImGuiDataTypeStorage; // Will rename in imgui/ later
void GetSliderTestRanges(ImGuiDataType data_type, ImGuiDataTypeStorage* min_p, ImGuiDataTypeStorage* max_p)
{
    switch (data_type)
    {
    case ImGuiDataType_S8:
        *(int8_t*)(void*)min_p = INT8_MIN;
        *(int8_t*)(void*)max_p = INT8_MAX;
        break;
    case ImGuiDataType_U8:
        *(uint8_t*)(void*)min_p = 0;
        *(uint8_t*)(void*)max_p = UINT8_MAX;
        break;
    case ImGuiDataType_S16:
        *(int16_t*)(void*)min_p = INT16_MIN;
        *(int16_t*)(void*)max_p = INT16_MAX;
        break;
    case ImGuiDataType_U16:
        *(uint16_t*)(void*)min_p = 0;
        *(uint16_t*)(void*)max_p = UINT16_MAX;
        break;
    case ImGuiDataType_S32:
        *(int32_t*)(void*)min_p = INT32_MIN / 2;
        *(int32_t*)(void*)max_p = INT32_MAX / 2;
        break;
    case ImGuiDataType_U32:
        *(uint32_t*)(void*)min_p = 0;
        *(uint32_t*)(void*)max_p = UINT32_MAX / 2;
        break;
    case ImGuiDataType_S64:
        *(int64_t*)(void*)min_p = INT64_MIN / 2;
        *(int64_t*)(void*)max_p = INT64_MAX / 2;
        break;
    case ImGuiDataType_U64:
        *(uint64_t*)(void*)min_p = 0;
        *(uint64_t*)(void*)max_p = UINT64_MAX / 2;
        break;
    case ImGuiDataType_Float:
        *(float*)(void*)min_p = -1000000000.0f; // Floating point types do not use their min/max supported values because widgets
        *(float*)(void*)max_p = +1000000000.0f; // may not be able to display them due to lossy RoundScalarWithFormatT().
        break;
    case ImGuiDataType_Double:
        *(double*)(void*)min_p = -1000000000.0;
        *(double*)(void*)max_p = +1000000000.0;
        break;
    default:
        IM_ASSERT(0);
    }
}

//-------------------------------------------------------------------------
// Tests: Widgets
//-------------------------------------------------------------------------

void RegisterTests_Widgets(ImGuiTestEngine* e)
{
    ImGuiTest* t = NULL;

    // ## Test basic button presses
    t = IM_REGISTER_TEST(e, "widgets", "widgets_button_press");
    struct ButtonPressTestVars { int ButtonPressCount[6] = { 0 }; };
    t->SetUserDataType<ButtonPressTestVars>();
    t->GuiFunc = [](ImGuiTestContext* ctx)
    {
        ButtonPressTestVars& vars = ctx->GetUserData<ButtonPressTestVars>();

        ImGui::Begin("Test Window", NULL, ImGuiWindowFlags_NoSavedSettings);
        if (ImGui::Button("Button0"))
            vars.ButtonPressCount[0]++;
        if (ImGui::ButtonEx("Button1", ImVec2(0, 0), ImGuiButtonFlags_PressedOnDoubleClick))
            vars.ButtonPressCount[1]++;
        if (ImGui::ButtonEx("Button2", ImVec2(0, 0), ImGuiButtonFlags_PressedOnClickRelease | ImGuiButtonFlags_PressedOnDoubleClick))
            vars.ButtonPressCount[2]++;
        if (ImGui::ButtonEx("Button3", ImVec2(0, 0), ImGuiButtonFlags_PressedOnClickReleaseAnywhere))
            vars.ButtonPressCount[3]++;
        if (ImGui::ButtonEx("Button4", ImVec2(0, 0), ImGuiButtonFlags_Repeat))
            vars.ButtonPressCount[4]++;
        ImGui::End();
    };
    t->TestFunc = [](ImGuiTestContext* ctx)
    {
        ButtonPressTestVars& vars = ctx->GetUserData<ButtonPressTestVars>();

        ctx->SetRef("Test Window");
        ctx->ItemClick("Button0");
        IM_CHECK_EQ(vars.ButtonPressCount[0], 1);
        ctx->ItemDoubleClick("Button1");
        IM_CHECK_EQ(vars.ButtonPressCount[1], 1);
        ctx->ItemDoubleClick("Button2");
        IM_CHECK_EQ(vars.ButtonPressCount[2], 2);

        // Test ImGuiButtonFlags_PressedOnClickRelease vs ImGuiButtonFlags_PressedOnClickReleaseAnywhere
        vars.ButtonPressCount[2] = 0;
        ctx->MouseMove("Button2");
        ctx->MouseDown(0);
        ctx->MouseMove("Button0", ImGuiTestOpFlags_NoCheckHoveredId);
        ctx->MouseUp(0);
        IM_CHECK_EQ(vars.ButtonPressCount[2], 0);
        ctx->MouseMove("Button3");
        ctx->MouseDown(0);
        ctx->MouseMove("Button0", ImGuiTestOpFlags_NoCheckHoveredId);
        ctx->MouseUp(0);
        IM_CHECK_EQ(vars.ButtonPressCount[3], 1);

        // Test ImGuiButtonFlags_Repeat
        ctx->ItemClick("Button4");
        IM_CHECK_EQ(vars.ButtonPressCount[4], 1);
        ctx->MouseDown(0);
        IM_CHECK_EQ(vars.ButtonPressCount[4], 1);
        const float step = ImMin(ctx->UiContext->IO.KeyRepeatDelay, ctx->UiContext->IO.KeyRepeatRate) * 0.50f;
        ctx->SleepNoSkip(ctx->UiContext->IO.KeyRepeatDelay, step);
        ctx->SleepNoSkip(ctx->UiContext->IO.KeyRepeatRate, step);
        ctx->SleepNoSkip(ctx->UiContext->IO.KeyRepeatRate, step);
        ctx->SleepNoSkip(ctx->UiContext->IO.KeyRepeatRate, step);
        IM_CHECK_EQ(vars.ButtonPressCount[4], 1 + 1 + 3 * 2); // FIXME: MouseRepeatRate is double KeyRepeatRate, that's not documented / or that's a bug
        ctx->MouseUp(0);
    };

    // ## Test basic button presses
    t = IM_REGISTER_TEST(e, "widgets", "widgets_button_mouse_buttons");
    t->SetUserDataType<ButtonPressTestVars>();
    t->GuiFunc = [](ImGuiTestContext* ctx)
    {
        ButtonPressTestVars& vars = ctx->GetUserData<ButtonPressTestVars>();

        ImGui::Begin("Test Window", NULL, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_AlwaysAutoResize);
        if (ImGui::ButtonEx("ButtonL", ImVec2(0, 0), ImGuiButtonFlags_MouseButtonLeft))
            vars.ButtonPressCount[0]++;
        if (ImGui::ButtonEx("ButtonR", ImVec2(0, 0), ImGuiButtonFlags_MouseButtonRight))
            vars.ButtonPressCount[1]++;
        if (ImGui::ButtonEx("ButtonM", ImVec2(0, 0), ImGuiButtonFlags_MouseButtonMiddle))
            vars.ButtonPressCount[2]++;
        if (ImGui::ButtonEx("ButtonLR", ImVec2(0, 0), ImGuiButtonFlags_MouseButtonLeft | ImGuiButtonFlags_MouseButtonRight))
            vars.ButtonPressCount[3]++;

        if (ImGui::ButtonEx("ButtonL-release", ImVec2(0, 0), ImGuiButtonFlags_MouseButtonLeft | ImGuiButtonFlags_PressedOnRelease))
            vars.ButtonPressCount[4]++;
        if (ImGui::ButtonEx("ButtonR-release", ImVec2(0, 0), ImGuiButtonFlags_MouseButtonRight | ImGuiButtonFlags_PressedOnRelease))
        {
            ctx->LogDebug("Pressed!");
            vars.ButtonPressCount[5]++;
        }
        for (int n = 0; n < IM_ARRAYSIZE(vars.ButtonPressCount); n++)
            ImGui::Text("%d: %d", n, vars.ButtonPressCount[n]);

        ImGui::End();
    };
    t->TestFunc = [](ImGuiTestContext* ctx)
    {
        ButtonPressTestVars& vars = ctx->GetUserData<ButtonPressTestVars>();

        ctx->SetRef("Test Window");
        ctx->ItemClick("ButtonL", 0);
        IM_CHECK_EQ(vars.ButtonPressCount[0], 1);
        ctx->ItemClick("ButtonR", 1);
        IM_CHECK_EQ(vars.ButtonPressCount[1], 1);
        ctx->ItemClick("ButtonM", 2);
        IM_CHECK_EQ(vars.ButtonPressCount[2], 1);
        ctx->ItemClick("ButtonLR", 0);
        ctx->ItemClick("ButtonLR", 1);
        IM_CHECK_EQ(vars.ButtonPressCount[3], 2);

        vars.ButtonPressCount[3] = 0;
        ctx->MouseMove("ButtonLR");
        ctx->MouseDown(0);
        ctx->MouseDown(1);
        ctx->MouseUp(0);
        ctx->MouseUp(1);
        IM_CHECK_EQ(vars.ButtonPressCount[3], 1);

        vars.ButtonPressCount[3] = 0;
        ctx->MouseMove("ButtonLR");
        ctx->MouseDown(0);
        ctx->MouseMove("ButtonR", ImGuiTestOpFlags_NoCheckHoveredId);
        ctx->MouseDown(1);
        ctx->MouseUp(0);
        ctx->MouseMove("ButtonLR");
        ctx->MouseUp(1);
        IM_CHECK_EQ(vars.ButtonPressCount[3], 0);
    };

    // ## Test ButtonBehavior frame by frame behaviors (see comments at the top of the ButtonBehavior() function)
    enum ButtonStateMachineTestStep
    {
        ButtonStateMachineTestStep_None,
        ButtonStateMachineTestStep_Init,
        ButtonStateMachineTestStep_MovedOver,
        ButtonStateMachineTestStep_MouseDown,
        ButtonStateMachineTestStep_MovedAway,
        ButtonStateMachineTestStep_MovedOverAgain,
        ButtonStateMachineTestStep_MouseUp,
        ButtonStateMachineTestStep_Done
    };
    t = IM_REGISTER_TEST(e, "widgets", "widgets_button_status");
    struct ButtonStateTestVars
    {
        ButtonStateMachineTestStep NextStep;
        ImGuiTestGenericStatus Status;
        ButtonStateTestVars() { NextStep = ButtonStateMachineTestStep_None; }
    };
    t->SetUserDataType<ButtonStateTestVars>();
    t->GuiFunc = [](ImGuiTestContext* ctx)
    {
        ButtonStateTestVars& vars = ctx->GetUserData<ButtonStateTestVars>();
        ImGuiTestGenericStatus& status = vars.Status;

        ImGui::Begin("Test Window", NULL, ImGuiWindowFlags_NoSavedSettings);

        const bool pressed = ImGui::Button("Test");
        status.QuerySet();
        switch (vars.NextStep)
        {
        case ButtonStateMachineTestStep_Init:
            IM_CHECK(0 == pressed);
            IM_CHECK(0 == status.Hovered);
            IM_CHECK(0 == status.Active);
            IM_CHECK(0 == status.Activated);
            IM_CHECK(0 == status.Deactivated);
            break;
        case ButtonStateMachineTestStep_MovedOver:
            IM_CHECK(0 == pressed);
            IM_CHECK(1 == status.Hovered);
            IM_CHECK(0 == status.Active);
            IM_CHECK(0 == status.Activated);
            IM_CHECK(0 == status.Deactivated);
            break;
        case ButtonStateMachineTestStep_MouseDown:
            IM_CHECK(0 == pressed);
            IM_CHECK(1 == status.Hovered);
            IM_CHECK(1 == status.Active);
            IM_CHECK(1 == status.Activated);
            IM_CHECK(0 == status.Deactivated);
            break;
        case ButtonStateMachineTestStep_MovedAway:
            IM_CHECK(0 == pressed);
            IM_CHECK(0 == status.Hovered);
            IM_CHECK(1 == status.Active);
            IM_CHECK(0 == status.Activated);
            IM_CHECK(0 == status.Deactivated);
            break;
        case ButtonStateMachineTestStep_MovedOverAgain:
            IM_CHECK(0 == pressed);
            IM_CHECK(1 == status.Hovered);
            IM_CHECK(1 == status.Active);
            IM_CHECK(0 == status.Activated);
            IM_CHECK(0 == status.Deactivated);
            break;
        case ButtonStateMachineTestStep_MouseUp:
            IM_CHECK(1 == pressed);
            IM_CHECK(1 == status.Hovered);
            IM_CHECK(0 == status.Active);
            IM_CHECK(0 == status.Activated);
            IM_CHECK(1 == status.Deactivated);
            break;
        case ButtonStateMachineTestStep_Done:
            IM_CHECK(0 == pressed);
            IM_CHECK(0 == status.Hovered);
            IM_CHECK(0 == status.Active);
            IM_CHECK(0 == status.Activated);
            IM_CHECK(0 == status.Deactivated);
            break;
        default:
            break;
        }
        vars.NextStep = ButtonStateMachineTestStep_None;

        // The "Unused" button allows to move the mouse away from the "Test" button
        ImGui::Button("Unused");

        ImGui::End();
    };
    t->TestFunc = [](ImGuiTestContext* ctx)
    {
        ButtonStateTestVars& vars = ctx->GetUserData<ButtonStateTestVars>();
        vars.NextStep = ButtonStateMachineTestStep_None;

        ctx->SetRef("Test Window");

        // Move mouse away from "Test" button
        ctx->MouseMove("Unused");
        vars.NextStep = ButtonStateMachineTestStep_Init;
        ctx->Yield();

        ctx->MouseMove("Test");
        vars.NextStep = ButtonStateMachineTestStep_MovedOver;
        ctx->Yield();

        vars.NextStep = ButtonStateMachineTestStep_MouseDown;
        ctx->MouseDown();

        ctx->MouseMove("Unused", ImGuiTestOpFlags_NoCheckHoveredId);
        vars.NextStep = ButtonStateMachineTestStep_MovedAway;
        ctx->Yield();

        ctx->MouseMove("Test");
        vars.NextStep = ButtonStateMachineTestStep_MovedOverAgain;
        ctx->Yield();

        vars.NextStep = ButtonStateMachineTestStep_MouseUp;
        ctx->MouseUp();

        ctx->MouseMove("Unused");
        vars.NextStep = ButtonStateMachineTestStep_Done;
        ctx->Yield();
    };

    // ## Test checkbox click
    t = IM_REGISTER_TEST(e, "widgets", "widgets_checkbox_001");
    t->GuiFunc = [](ImGuiTestContext* ctx)
    {
        ImGui::Begin("Window1", NULL, ImGuiWindowFlags_NoSavedSettings);
        ImGui::Checkbox("Checkbox", &ctx->GenericVars.Bool1);
        ImGui::End();
    };
    t->TestFunc = [](ImGuiTestContext* ctx)
    {
        // We use WindowRef() to ensure the window is uncollapsed.
        IM_CHECK(ctx->GenericVars.Bool1 == false);
        ctx->SetRef("Window1");
        ctx->ItemClick("Checkbox");
        IM_CHECK(ctx->GenericVars.Bool1 == true);
    };

    // ## Test all types with DragScalar().
    t = IM_REGISTER_TEST(e, "widgets", "widgets_datatype_1");
    struct DragDatatypeVars { int widget_type = 0;  ImGuiDataType data_type = 0; char data_storage[10] = ""; char data_zero[8] = ""; };
    t->SetUserDataType<DragDatatypeVars>();
    t->GuiFunc = [](ImGuiTestContext* ctx)
    {
        DragDatatypeVars& vars = ctx->GetUserData<DragDatatypeVars>();
        ImGui::SetNextWindowSize(ImVec2(200, 200));
        ImGui::Begin("Test Window", NULL, ImGuiWindowFlags_NoSavedSettings);
        bool ret;
        if (vars.widget_type == 0)
            ret = ImGui::DragScalar("Drag", vars.data_type, &vars.data_storage[1], 0.5f);
        else
            ret = ImGui::SliderScalar("Slider", vars.data_type, &vars.data_storage[1], &vars.data_zero, &vars.data_zero);
        ctx->GenericVars.Status.QueryInc(ret);
        ImGui::End();
    };
    t->TestFunc = [](ImGuiTestContext* ctx)
    {
        ImGuiContext& g = *ctx->UiContext;
        DragDatatypeVars& vars = ctx->GetUserData<DragDatatypeVars>();

        ctx->SetRef("Test Window");
        for (int widget_type = 0; widget_type < 2; widget_type++)
        {
            for (int data_type = 0; data_type < ImGuiDataType_COUNT; data_type++)
            {
                size_t data_size = ImGui::DataTypeGetInfo(data_type)->Size;
                IM_ASSERT(data_size + 2 <= sizeof(vars.data_storage));
                memset(vars.data_storage, 0, sizeof(vars.data_storage));
                memset(vars.data_zero, 0, sizeof(vars.data_zero));
                vars.widget_type = widget_type;
                vars.data_type = data_type;
                vars.data_storage[0] = vars.data_storage[1 + data_size] = 42; // Sentinel values
                const char* widget_name = widget_type == 0 ? "Drag" : "Slider";

                if (widget_type == 0)
                {
                    ctx->GenericVars.Status.Clear();
                    ctx->MouseMove(widget_name);
                    ctx->MouseDown();
                    ctx->MouseMoveToPos(g.IO.MousePos + ImVec2(30, 0));
                    IM_CHECK(ctx->GenericVars.Status.Edited >= 1);
                    ctx->GenericVars.Status.Clear();
                    ctx->MouseMoveToPos(g.IO.MousePos + ImVec2(-40, 0));
                    IM_CHECK(ctx->GenericVars.Status.Edited >= 1);
                    ctx->MouseUp();
                }

                ctx->GenericVars.Status.Clear();
                ctx->ItemInput(widget_name);
                ctx->KeyChars("123");                               // Case fixed by PR #3231
                IM_CHECK_GE(ctx->GenericVars.Status.Ret, 1);
                IM_CHECK_GE(ctx->GenericVars.Status.Edited, 1);
                ctx->GenericVars.Status.Clear();
                ctx->Yield();
                IM_CHECK_EQ(ctx->GenericVars.Status.Ret, 0);        // Verify it doesn't keep returning as edited.
                IM_CHECK_EQ(ctx->GenericVars.Status.Edited, 0);

                ctx->GenericVars.Status.Clear();
                ctx->KeyPressMap(ImGuiKey_Enter);
                IM_CHECK(vars.data_storage[0] == 42);               // Ensure there were no oob writes.
                IM_CHECK(vars.data_storage[1 + data_size] == 42);
            }
        }
    };

    // ## Test DragInt() as InputText
    // ## Test ColorEdit4() as InputText (#2557)
    t = IM_REGISTER_TEST(e, "widgets", "widgets_as_input");
    t->GuiFunc = [](ImGuiTestContext* ctx)
    {
        ImGuiTestGenericVars& vars = ctx->GenericVars;
        ImGui::Begin("Test Window", NULL, ImGuiWindowFlags_NoSavedSettings);
        ImGui::DragInt("Drag", &vars.Int1);
        ImGui::ColorEdit4("Color", &vars.Vec4.x);
        ImGui::End();
    };
    t->TestFunc = [](ImGuiTestContext* ctx)
    {
        ImGuiTestGenericVars& vars = ctx->GenericVars;
        ctx->SetRef("Test Window");

        IM_CHECK_EQ(vars.Int1, 0);
        ctx->ItemInput("Drag");
        IM_CHECK_EQ(ctx->UiContext->ActiveId, ctx->GetID("Drag"));
        ctx->KeyCharsAppendEnter("123");
        IM_CHECK_EQ(vars.Int1, 123);

        ctx->ItemInput("Color##Y");
        IM_CHECK_EQ(ctx->UiContext->ActiveId, ctx->GetID("Color##Y"));
        ctx->KeyCharsAppend("123");
        IM_CHECK(ImFloatEq(vars.Vec4.y, 123.0f / 255.0f));
        ctx->KeyPressMap(ImGuiKey_Tab);
        ctx->KeyCharsAppendEnter("200");
        IM_CHECK(ImFloatEq(vars.Vec4.x,   0.0f / 255.0f));
        IM_CHECK(ImFloatEq(vars.Vec4.y, 123.0f / 255.0f));
        IM_CHECK(ImFloatEq(vars.Vec4.z, 200.0f / 255.0f));
    };

    // ## Test Sliders and Drags clamping values
    t = IM_REGISTER_TEST(e, "widgets", "widgets_drag_slider_clamping");
    struct ImGuiDragSliderVars { float DragValue = 0.0f; float DragMin = 0.0f; float DragMax = 1.0f; float SliderValue = 0.0f; float SliderMin = 0.0f; float SliderMax = 0.0f; float ScalarValue = 0.0f; void* ScalarMinP = NULL; void* ScalarMaxP = NULL; ImGuiSliderFlags Flags = ImGuiSliderFlags_None; };
    t->SetUserDataType<ImGuiDragSliderVars>();
    t->GuiFunc = [](ImGuiTestContext* ctx)
    {
        ImGuiDragSliderVars& vars = ctx->GetUserData<ImGuiDragSliderVars>();
        ImGui::Begin("Test Window", NULL, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_AlwaysAutoResize);
        const char* format = "%.3f";
        ImGui::SliderFloat("Slider", &vars.SliderValue, vars.SliderMin, vars.SliderMax, format, vars.Flags);
        ImGui::DragFloat("Drag", &vars.DragValue, 1.0f, vars.DragMin, vars.DragMax, format, vars.Flags);
        ImGui::DragScalar("Scalar", ImGuiDataType_Float, &vars.ScalarValue, 1.0f, vars.ScalarMinP, vars.ScalarMaxP, NULL, vars.Flags);
        ImGui::End();
    };
    t->TestFunc = [](ImGuiTestContext* ctx)
    {
        ImGuiContext& g = *ImGui::GetCurrentContext();
        ImGuiDragSliderVars& vars = ctx->GetUserData<ImGuiDragSliderVars>();
        ctx->SetRef("Test Window");
        ImGuiSliderFlags flags[] = { ImGuiSliderFlags_None, ImGuiSliderFlags_AlwaysClamp };
        for (int i = 0; i < IM_ARRAYSIZE(flags); ++i)
        {
            bool clamp_on_input = flags[i] == ImGuiSliderFlags_AlwaysClamp;
            vars.Flags = flags[i];

            float slider_min_max[][2] = { {0.0f, 1.0f}, {0.0f, 0.0f} };
            for (int j = 0; j < IM_ARRAYSIZE(slider_min_max); ++j)
            {
                ctx->LogInfo("## Slider %d with Flags = 0x%08X", j, vars.Flags);

                vars.SliderValue = 0.0f;
                vars.SliderMin = slider_min_max[j][0];
                vars.SliderMax = slider_min_max[j][1];

                ctx->ItemInput("Slider");
                ctx->KeyCharsReplaceEnter("2");
                IM_CHECK_EQ(vars.SliderValue, clamp_on_input ? vars.SliderMax : 2.0f);

                // Check higher bound
                ctx->MouseMove("Slider", ImGuiTestOpFlags_MoveToEdgeR);
                ctx->MouseDown(); // Click will update clamping
                IM_CHECK_EQ(vars.SliderValue, vars.SliderMax);
                ctx->MouseMoveToPos(g.IO.MousePos + ImVec2(100, 0));
                ctx->MouseUp();
                IM_CHECK_EQ(vars.SliderValue, vars.SliderMax);

                ctx->ItemInput("Slider");
                ctx->KeyCharsReplaceEnter("-2");
                IM_CHECK_EQ(vars.SliderValue, clamp_on_input ? vars.SliderMin : -2.0f);

                // Check lower bound
                ctx->MouseMove("Slider", ImGuiTestOpFlags_MoveToEdgeL);
                ctx->MouseDown(); // Click will update clamping
                IM_CHECK_EQ(vars.SliderValue, vars.SliderMin);
                ctx->MouseMoveToPos(g.IO.MousePos - ImVec2(100, 0));
                ctx->MouseUp();
                IM_CHECK_EQ(vars.SliderValue, vars.SliderMin);
            }

            float drag_min_max[][2] = { {0.0f, 1.0f}, {0.0f, 0.0f}, {-FLT_MAX, FLT_MAX} };
            for (int j = 0; j < IM_ARRAYSIZE(drag_min_max); ++j)
            {
                ctx->LogDebug("Drag %d with flags = 0x%08X", j, vars.Flags);

                vars.DragValue = 0.0f;
                vars.DragMin = drag_min_max[j][0];
                vars.DragMax = drag_min_max[j][1];

                // [0,0] is equivalent to [-FLT_MAX, FLT_MAX] range
                bool unbound = (vars.DragMin == 0.0f && vars.DragMax == 0.0f) || (vars.DragMin == -FLT_MAX && vars.DragMax == FLT_MAX);
                float value_before_click = 0.0f;

                ctx->ItemInput("Drag");
                ctx->KeyCharsReplaceEnter("-3");
                IM_CHECK_EQ(vars.DragValue, clamp_on_input && !unbound ? vars.DragMin : -3.0f);

                ctx->ItemInput("Drag");
                ctx->KeyCharsReplaceEnter("2");
                IM_CHECK_EQ(vars.DragValue, clamp_on_input && !unbound ? vars.DragMax : 2.0f);

                // Check higher bound
                ctx->MouseMove("Drag");
                value_before_click = vars.DragValue;
                ctx->MouseDown(); // Click will not update clamping value
                IM_CHECK_EQ(vars.DragValue, value_before_click);
                ctx->MouseMoveToPos(g.IO.MousePos + ImVec2(100, 0));
                ctx->MouseUp();
                if (unbound)
                    IM_CHECK_GT(vars.DragValue, value_before_click);
                else
                    IM_CHECK_EQ(vars.DragValue, value_before_click);

                // Check higher to lower bound
                value_before_click = vars.DragValue;
                ctx->MouseMove("Drag");
                ctx->MouseDragWithDelta(ImVec2(-100, 0));
                if (unbound)
                    IM_CHECK_LT(vars.DragValue, value_before_click);
                else
                    IM_CHECK_EQ(vars.DragValue, vars.DragMin);

                // Check low to high bound
                value_before_click = vars.DragValue;
                ctx->MouseMove("Drag");
                ctx->MouseDragWithDelta(ImVec2(100, 0));
                if (unbound)
                    IM_CHECK_GT(vars.DragValue, value_before_click);
                else
                    IM_CHECK_EQ(vars.DragValue, vars.DragMax);
            }

            float scalar_min_max[][2] = { {-FLT_MAX, 1.0f}, {0.0f, FLT_MAX} };
            for (int j = 0; j < IM_ARRAYSIZE(scalar_min_max); ++j)
            {
                ctx->LogDebug("Scalar %d with flags = 0x%08X", j, vars.Flags);

                vars.ScalarValue = 0.0f;
                vars.DragMin = scalar_min_max[j][0];    // No harm in reusing DragMin/DragMax.
                vars.DragMax = scalar_min_max[j][1];
                vars.ScalarMinP = (vars.DragMin == -FLT_MAX || vars.DragMin == +FLT_MAX) ? NULL : &vars.DragMin;
                vars.ScalarMaxP = (vars.DragMax == -FLT_MAX || vars.DragMax == +FLT_MAX) ? NULL : &vars.DragMax;

                bool unbound_min = vars.ScalarMinP == NULL;
                bool unbound_max = vars.ScalarMaxP == NULL;
                float value_before_click = 0.0f;

                ctx->ItemInput("Scalar");
                ctx->KeyCharsReplaceEnter("-3");
                IM_CHECK_EQ(vars.ScalarValue, clamp_on_input && !unbound_min ? vars.DragMin : -3.0f);

                ctx->ItemInput("Scalar");
                ctx->KeyCharsReplaceEnter("2");
                IM_CHECK_EQ(vars.ScalarValue, clamp_on_input && !unbound_max ? vars.DragMax : 2.0f);

                // Check higher bound
                ctx->MouseMove("Scalar");
                value_before_click = vars.ScalarValue;
                ctx->MouseDown(); // Click will not update clamping value
                IM_CHECK_EQ(vars.ScalarValue, value_before_click);
                ctx->MouseMoveToPos(g.IO.MousePos + ImVec2(100, 0));
                ctx->MouseUp();
                if (unbound_max)
                    IM_CHECK_GT(vars.ScalarValue, value_before_click);
                else
                    IM_CHECK_EQ(vars.ScalarValue, value_before_click);

                // Check higher to lower bound
                value_before_click = vars.ScalarValue = 50.0f;
                ctx->MouseMove("Scalar");
                ctx->MouseDragWithDelta(ImVec2(-100, 0));
                if (unbound_min)
                    IM_CHECK_LT(vars.ScalarValue, value_before_click);
                else
                    IM_CHECK_EQ(vars.ScalarValue, vars.DragMin);

                // Check low to high bound
                value_before_click = vars.ScalarValue = 0.0f;
                ctx->MouseMove("Scalar");
                ctx->MouseDragWithDelta(ImVec2(100, 0));
                if (unbound_max)
                    IM_CHECK_GT(vars.ScalarValue, value_before_click);
                else
                    IM_CHECK_EQ(vars.ScalarValue, vars.DragMax);
            }
        }
    };

#if 0
    // ## Testing Scroll on Window with Test Engine helpers
    t = IM_REGISTER_TEST(e, "widgets", "widgets_scrollbar");
    t->GuiFunc = [](ImGuiTestContext* ctx)
    {
        ImGui::SetNextWindowSize(ImVec2(200, 200));
        ImGui::Begin("Test Window", NULL, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_AlwaysHorizontalScrollbar);
        ImGui::BeginChild("##Child", ImVec2(800, 800), true);
        ImGui::Text("This is some text");
        ImGui::EndChild();
        ImGui::End();
    };
    t->TestFunc = [](ImGuiTestContext* ctx)
    {
        ctx->SetRef("Test Window");
        ctx->ScrollTo(ImGuiAxis_Y, 60);
        ctx->ScrollTo(ImGuiAxis_X, 80);
    };
#endif

    // ## Test for IsItemHovered() on BeginChild() and InputMultiLine() (#1370, #3851)
    t = IM_REGISTER_TEST(e, "widgets", "widgets_hover");
    t->GuiFunc = [](ImGuiTestContext* ctx)
    {
        ImGuiTestGenericVars& vars = ctx->GenericVars;
        ImGui::Begin("Test Window", NULL, ImGuiWindowFlags_NoSavedSettings);

        ImGui::BeginChild("Child", ImVec2(100, 100), true);
        vars.Pos = ImGui::GetCursorScreenPos();
        ImGui::Dummy(ImVec2(10, 10));
        ImGui::Button("button");
        vars.Id = ImGui::GetItemID();
        ImGui::EndChild();
        vars.Bool1 = ImGui::IsItemHovered();
        ImGui::Text("hovered: %d", vars.Bool1);

        ImGui::InputTextMultiline("##Field", vars.Str1, IM_ARRAYSIZE(vars.Str1), ImVec2(-FLT_MIN, 0.0f));
        vars.Bool2 = ImGui::IsItemHovered();
        ImGui::Text("hovered: %d", vars.Bool2);
        ImGui::End();
    };
    t->TestFunc = [](ImGuiTestContext* ctx)
    {
        ImGuiTestGenericVars& vars = ctx->GenericVars;
        ctx->SetRef("Test Window");

        // move on child's void
        ctx->MouseMoveToPos(vars.Pos);
        IM_CHECK(vars.Bool1 == true);
        IM_CHECK(vars.Bool2 == false);

        // move on child's item
        ctx->MouseMove(vars.Id);
        IM_CHECK(vars.Bool1 == true);
        IM_CHECK(vars.Bool2 == false);

        // move on InputTextMultiline() = child embedded in group
        ctx->MouseMove("##Field");
        IM_CHECK(vars.Bool1 == false);
        IM_CHECK(vars.Bool2 == true);
    };

    // ## Test InputText widget
    t = IM_REGISTER_TEST(e, "widgets", "widgets_inputtext_basic");
    t->GuiFunc = [](ImGuiTestContext* ctx)
    {
        ImGuiTestGenericVars& vars = ctx->GenericVars;
        ImGui::SetNextWindowSize(ImVec2(200, 200));
        ImGui::Begin("Test Window", NULL, ImGuiWindowFlags_NoSavedSettings);
        ImGui::InputText("InputText", vars.Str1, IM_ARRAYSIZE(vars.Str1), vars.Bool1 ? ImGuiInputTextFlags_ReadOnly : ImGuiInputTextFlags_None);
        ImGui::End();
    };
    t->TestFunc = [](ImGuiTestContext* ctx)
    {
        ImGuiInputTextState& state = ctx->UiContext->InputTextState;
        char* buf = ctx->GenericVars.Str1;

        ctx->SetRef("Test Window");

        // Insert
        strcpy(buf, "Hello");
        ctx->ItemClick("InputText");
        ctx->KeyCharsAppendEnter(u8"World123\u00A9");
        IM_CHECK_STR_EQ(buf, u8"HelloWorld123\u00A9");
        IM_CHECK_EQ(state.CurLenA, 15);
        IM_CHECK_EQ(state.CurLenW, 14);

        // Delete
        ctx->ItemClick("InputText");
        ctx->KeyPressMap(ImGuiKey_End);
        ctx->KeyPressMap(ImGuiKey_LeftArrow, ImGuiKeyModFlags_Shift, 2);    // Select last two characters
        ctx->KeyPressMap(ImGuiKey_Backspace, ImGuiKeyModFlags_None, 3);     // Delete selection and two more characters
        ctx->KeyPressMap(ImGuiKey_Enter);
        IM_CHECK_STR_EQ(buf, "HelloWorld");
        IM_CHECK_EQ(state.CurLenA, 10);
        IM_CHECK_EQ(state.CurLenW, 10);

        // Insert, Cancel
        ctx->ItemClick("InputText");
        ctx->KeyPressMap(ImGuiKey_End);
        ctx->KeyChars("XXXXX");
        ctx->KeyPressMap(ImGuiKey_Escape);
        IM_CHECK_STR_EQ(buf, "HelloWorld");
        IM_CHECK_EQ(state.CurLenA, 10);
        IM_CHECK_EQ(state.CurLenW, 10);

        // Delete, Cancel
        ctx->ItemClick("InputText");
        ctx->KeyPressMap(ImGuiKey_End);
        ctx->KeyPressMap(ImGuiKey_Backspace, ImGuiKeyModFlags_None, 5);
        ctx->KeyPressMap(ImGuiKey_Escape);
        IM_CHECK_STR_EQ(buf, "HelloWorld");
        IM_CHECK_EQ(state.CurLenA, 10);
        IM_CHECK_EQ(state.CurLenW, 10);

        // Readonly mode
        ImGuiTestGenericVars& vars = ctx->GenericVars;
        strcpy(buf, "Some read-only text.");
        vars.Bool1 = true;
        ctx->Yield();

        ctx->ItemClick("InputText");
        ctx->KeyCharsAppendEnter("World123");
        IM_CHECK_STR_EQ(buf, vars.Str1);
        IM_CHECK_EQ(state.CurLenA, 20);
        IM_CHECK_EQ(state.CurLenW, 20);
    };

    // ## Test InputText undo/redo ops, in particular related to issue we had with stb_textedit undo/redo buffers
    t = IM_REGISTER_TEST(e, "widgets", "widgets_inputtext_undo_redo");
    t->GuiFunc = [](ImGuiTestContext* ctx)
    {
        ImGuiTestGenericVars& vars = ctx->GenericVars;
        if (vars.StrLarge.empty())
            vars.StrLarge.resize(10000, 0);
        ImGui::SetNextWindowSize(ImVec2(ImGui::GetFontSize() * 50, 0.0f));
        ImGui::Begin("Test Window", NULL, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_AlwaysAutoResize);
        ImGui::Text("strlen() = %d", (int)strlen(vars.StrLarge.Data));
        ImGui::InputText("Other", vars.Str1, IM_ARRAYSIZE(vars.Str1), ImGuiInputTextFlags_None);
        ImGui::InputTextMultiline("InputText", vars.StrLarge.Data, (size_t)vars.StrLarge.Size, ImVec2(-1, ImGui::GetFontSize() * 20), ImGuiInputTextFlags_None);
        ImGui::End();
        //DebugInputTextState();
    };
    t->TestFunc = [](ImGuiTestContext* ctx)
    {
        // https://github.com/nothings/stb/issues/321
        ImGuiTestGenericVars& vars = ctx->GenericVars;
        ImGuiContext& g = *ctx->UiContext;

        // Start with a 350 characters buffer.
        // For this test we don't inject the characters via pasting or key-by-key in order to precisely control the undo/redo state.
        char* buf = vars.StrLarge.Data;
        IM_CHECK_EQ((int)strlen(buf), 0);
        for (int n = 0; n < 10; n++)
            strcat(buf, "xxxxxxx abcdefghijklmnopqrstuvwxyz\n");
        IM_CHECK_EQ((int)strlen(buf), 350);

        ctx->SetRef("Test Window");
        ctx->ItemClick("Other"); // This is to ensure stb_textedit_clear_state() gets called (clear the undo buffer, etc.)
        ctx->ItemClick("InputText");

        ImGuiInputTextState& input_text_state = g.InputTextState;
        ImStb::StbUndoState& undo_state = input_text_state.Stb.undostate;
        IM_CHECK_EQ(input_text_state.ID, g.ActiveId);
        IM_CHECK_EQ(undo_state.undo_point, 0);
        IM_CHECK_EQ(undo_state.undo_char_point, 0);
        IM_CHECK_EQ(undo_state.redo_point, STB_TEXTEDIT_UNDOSTATECOUNT);
        IM_CHECK_EQ(undo_state.redo_char_point, STB_TEXTEDIT_UNDOCHARCOUNT);
        IM_CHECK_EQ(STB_TEXTEDIT_UNDOCHARCOUNT, 999); // Test designed for this value

        // Insert 350 characters via 10 paste operations
        // We use paste operations instead of key-by-key insertion so we know our undo buffer will contains 10 undo points.
        //const char line_buf[26+8+1+1] = "xxxxxxx abcdefghijklmnopqrstuvwxyz\n"; // 8+26+1 = 35
        //ImGui::SetClipboardText(line_buf);
        //IM_CHECK(strlen(line_buf) == 35);
        //ctx->KeyPressMap(ImGuiKey_V, ImGuiKeyModFlags_Shortcut, 10);

        // Select all, copy, paste 3 times
        ctx->KeyPressMap(ImGuiKey_A, ImGuiKeyModFlags_Shortcut);    // Select all
        ctx->KeyPressMap(ImGuiKey_C, ImGuiKeyModFlags_Shortcut);    // Copy
        ctx->KeyPressMap(ImGuiKey_End, ImGuiKeyModFlags_Shortcut);  // Go to end, clear selection
        ctx->SleepShort();
        for (int n = 0; n < 3; n++)
        {
            ctx->KeyPressMap(ImGuiKey_V, ImGuiKeyModFlags_Shortcut);// Paste append three times
            ctx->SleepShort();
        }
        int len = (int)strlen(vars.StrLarge.Data);
        IM_CHECK_EQ(len, 350 * 4);
        IM_CHECK_EQ(undo_state.undo_point, 3);
        IM_CHECK_EQ(undo_state.undo_char_point, 0);

        // Undo x2
        IM_CHECK(undo_state.redo_point == STB_TEXTEDIT_UNDOSTATECOUNT);
        ctx->KeyPressMap(ImGuiKey_Z, ImGuiKeyModFlags_Shortcut);
        ctx->KeyPressMap(ImGuiKey_Z, ImGuiKeyModFlags_Shortcut);
        len = (int)strlen(vars.StrLarge.Data);
        IM_CHECK_EQ(len, 350 * 2);
        IM_CHECK_EQ(undo_state.undo_point, 1);
        IM_CHECK_EQ(undo_state.redo_point, STB_TEXTEDIT_UNDOSTATECOUNT - 2);
        IM_CHECK_EQ(undo_state.redo_char_point, STB_TEXTEDIT_UNDOCHARCOUNT - 350 * 2);

        // Undo x1 should call stb_textedit_discard_redo()
        ctx->KeyPressMap(ImGuiKey_Z, ImGuiKeyModFlags_Shortcut);
        len = (int)strlen(vars.StrLarge.Data);
        IM_CHECK_EQ(len, 350 * 1);
    };

    // ## Test InputText vs user ownership of data
    t = IM_REGISTER_TEST(e, "widgets", "widgets_inputtext_text_ownership");
    t->GuiFunc = [](ImGuiTestContext* ctx)
    {
        ImGuiTestGenericVars& vars = ctx->GenericVars;
        ImGui::Begin("Test Window", NULL, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_AlwaysAutoResize);
        ImGui::LogToBuffer();
        ImGui::InputText("##InputText", vars.Str1, IM_ARRAYSIZE(vars.Str1)); // Remove label to simplify the capture/comparison
        ImStrncpy(vars.Str2, ctx->UiContext->LogBuffer.c_str(), IM_ARRAYSIZE(vars.Str2));
        ImGui::LogFinish();
        ImGui::Text("Captured: \"%s\"", vars.Str2);
        ImGui::End();
    };
    t->TestFunc = [](ImGuiTestContext* ctx)
    {
        ImGuiTestGenericVars& vars = ctx->GenericVars;
        char* buf_user = vars.Str1;
        char* buf_visible = vars.Str2;
        ctx->SetRef("Test Window");

        IM_CHECK_STR_EQ(buf_visible, "{ }");
        strcpy(buf_user, "Hello");
        ctx->Yield();
        IM_CHECK_STR_EQ(buf_visible, "{ Hello }");
        ctx->ItemClick("##InputText");
        ctx->KeyCharsAppend("1");
        ctx->Yield();
        IM_CHECK_STR_EQ(buf_user, "Hello1");
        IM_CHECK_STR_EQ(buf_visible, "{ Hello1 }");

        // Because the item is active, it owns the source data, so:
        strcpy(buf_user, "Overwritten");
        ctx->Yield();
        IM_CHECK_STR_EQ(buf_user, "Hello1");
        IM_CHECK_STR_EQ(buf_visible, "{ Hello1 }");

        // Lose focus, at this point the InputTextState->ID should be holding on the last active state,
        // so we verify that InputText() is picking up external changes.
        ctx->KeyPressMap(ImGuiKey_Escape);
        IM_CHECK_EQ(ctx->UiContext->ActiveId, (unsigned)0);
        strcpy(buf_user, "Hello2");
        ctx->Yield();
        IM_CHECK_STR_EQ(buf_user, "Hello2");
        IM_CHECK_STR_EQ(buf_visible, "{ Hello2 }");
    };

    // ## Test that InputText doesn't go havoc when activated via another item
    t = IM_REGISTER_TEST(e, "widgets", "widgets_inputtext_id_conflict");
    t->GuiFunc = [](ImGuiTestContext* ctx)
    {
        ImGuiTestGenericVars& vars = ctx->GenericVars;
        ImGui::SetNextWindowSize(ImVec2(ImGui::GetFontSize() * 50, 0.0f));
        ImGui::Begin("Test Window", NULL, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_AlwaysAutoResize);
        if (vars.Step == 0)
        {
            if (ctx->FrameCount < 50)
                ImGui::Button("Hello");
            else
                ImGui::InputText("Hello", vars.Str1, IM_ARRAYSIZE(vars.Str1));
        }
        else
        {
            ImGui::InputTextMultiline("Hello", vars.Str1, IM_ARRAYSIZE(vars.Str1));
        }
        ImGui::End();
    };
    t->TestFunc = [](ImGuiTestContext* ctx)
    {
        ctx->SetRef("Test Window");
        ctx->ItemHoldForFrames("Hello", 100);
        ctx->ItemClick("Hello");
        ImGuiInputTextState* state = ImGui::GetInputTextState(ctx->GetID("Hello"));
        IM_CHECK(state != NULL);
        IM_CHECK(state->Stb.single_line == 1);

        // Toggling from single to multiline is a little bit ill-defined
        ctx->GenericVars.Step = 1;
        ctx->Yield();
        ctx->ItemClick("Hello");
        state = ImGui::GetInputTextState(ctx->GetID("Hello"));
        IM_CHECK(state != NULL);
        IM_CHECK(state->Stb.single_line == 0);
    };

    // ## Test that InputText doesn't append two tab characters if the backend supplies both tab key and character
    t = IM_REGISTER_TEST(e, "widgets", "widgets_inputtext_tab_double_insertion");
    t->GuiFunc = [](ImGuiTestContext* ctx)
    {
        ImGuiTestGenericVars& vars = ctx->GenericVars;
        ImGui::Begin("Test Window", NULL, ImGuiWindowFlags_NoSavedSettings);
        ImGui::InputText("Field", vars.Str1, IM_ARRAYSIZE(vars.Str1), ImGuiInputTextFlags_AllowTabInput);
        ImGui::End();
    };
    t->TestFunc = [](ImGuiTestContext* ctx)
    {
        ImGuiTestGenericVars& vars = ctx->GenericVars;
        ctx->SetRef("Test Window");
        ctx->ItemClick("Field");
        ctx->UiContext->IO.AddInputCharacter((ImWchar)'\t');
        ctx->KeyPressMap(ImGuiKey_Tab);
        IM_CHECK_STR_EQ(vars.Str1, "\t");
    };

    // ## Test input clearing action (ESC key) being undoable (#3008).
    t = IM_REGISTER_TEST(e, "widgets", "widgets_inputtext_esc_undo");
    t->GuiFunc = [](ImGuiTestContext* ctx)
    {
        ImGuiTestGenericVars& vars = ctx->GenericVars;
        ImGui::Begin("Test Window", NULL, ImGuiWindowFlags_NoSavedSettings);
        ImGui::InputText("Field", vars.Str1, IM_ARRAYSIZE(vars.Str1));
        ImGui::End();

    };
    t->TestFunc = [](ImGuiTestContext* ctx)
    {
        ImGuiTestGenericVars& vars = ctx->GenericVars;
        // FIXME-TESTS: Facilitate usage of variants
        const int test_count = ctx->HasDock ? 2 : 1;
        for (int test_n = 0; test_n < test_count; test_n++)
        {
            ctx->LogDebug("TEST CASE %d", test_n);
            const char* initial_value = (test_n == 0) ? "" : "initial";
            strcpy(vars.Str1, initial_value);
            ctx->SetRef("Test Window");
            ctx->ItemInput("Field");
            ctx->KeyCharsReplace("text");
            IM_CHECK_STR_EQ(vars.Str1, "text");
            ctx->KeyPressMap(ImGuiKey_Escape);                      // Reset input to initial value.
            IM_CHECK_STR_EQ(vars.Str1, initial_value);
            ctx->ItemInput("Field");
            ctx->KeyPressMap(ImGuiKey_Z, ImGuiKeyModFlags_Shortcut);    // Undo
            IM_CHECK_STR_EQ(vars.Str1, "text");
            ctx->KeyPressMap(ImGuiKey_Enter);                       // Unfocus otherwise test_n==1 strcpy will fail
        }
    };

    // ## Test input text multiline cursor movement: left, up, right, down, origin, end, ctrl+origin, ctrl+end, page up, page down
    // ## Verify that text selection does not leak spaces in password fields. (#4155)
    t = IM_REGISTER_TEST(e, "widgets", "widgets_inputtext_cursor");
    struct InputTextCursorVars { Str str; int Cursor = 0; int LineCount = 10; Str64 Password; };
    t->SetUserDataType<InputTextCursorVars>();
    t->GuiFunc = [](ImGuiTestContext* ctx)
    {
        InputTextCursorVars& vars = ctx->GetUserData<InputTextCursorVars>();

        float height = vars.LineCount * 0.5f * ImGui::GetFontSize();
        ImGui::Begin("Test Window", NULL, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_AlwaysAutoResize);
        ImGui::InputTextMultiline("Field", &vars.str, ImVec2(300, height), ImGuiInputTextFlags_EnterReturnsTrue);
        ImGui::InputText("Password", &vars.Password, ImGuiInputTextFlags_Password);
        if (ImGuiInputTextState* state = ImGui::GetInputTextState(ctx->GetID("/Test Window/Field")))
            ImGui::Text("Stb Cursor: %d", state->Stb.cursor);
        ImGui::End();
    };
    t->TestFunc = [](ImGuiTestContext* ctx)
    {
        InputTextCursorVars& vars = ctx->GetUserData<InputTextCursorVars>();
        ctx->SetRef("Test Window");

        vars.str.clear();
        const int char_count_per_line = 10;
        for (int n = 0; n < vars.LineCount; n++)
        {
            for (int c = 0; c < char_count_per_line - 1; c++) // \n is part of our char_count_per_line
                vars.str.appendf("%d", n);
            if (n < vars.LineCount - 1)
                vars.str.append("\n");
        }
        ctx->ItemInput("Field");

        ImGuiInputTextState* state = ImGui::GetInputTextState(ctx->GetID("Field"));
        IM_CHECK(state != NULL);
        ImStb::STB_TexteditState& stb = state->Stb;
        vars.Cursor = stb.cursor;

        const int page_size = (vars.LineCount / 2) - 1;

        const int cursor_pos_begin_of_first_line = 0;
        const int cursor_pos_end_of_first_line = char_count_per_line - 1;
        const int cursor_pos_middle_of_first_line = char_count_per_line / 2;
        const int cursor_pos_end_of_last_line = vars.str.length();
        const int cursor_pos_begin_of_last_line = cursor_pos_end_of_last_line - char_count_per_line + 1;
        //const int cursor_pos_middle_of_last_line = cursor_pos_end_of_last_line - char_count_per_line / 2;
        const int cursor_pos_middle = vars.str.length() / 2;

        auto SetCursorPosition = [&stb](int cursor) { stb.cursor = cursor; stb.has_preferred_x = 0; };

        // Do all the test twice: with no trailing \n, and with.
        for (int i = 0; i < 2; i++)
        {
            bool has_trailing_line_feed = (i == 1);
            if (has_trailing_line_feed)
            {
                SetCursorPosition(cursor_pos_end_of_last_line);
                ctx->KeyCharsAppend("\n");
            }
            int eof = vars.str.length();

            // Begin of File
            SetCursorPosition(0); ctx->KeyPressMap(ImGuiKey_UpArrow);
            IM_CHECK_EQ(stb.cursor, 0);
            SetCursorPosition(0); ctx->KeyPressMap(ImGuiKey_LeftArrow);
            IM_CHECK_EQ(stb.cursor, 0);
            SetCursorPosition(0); ctx->KeyPressMap(ImGuiKey_DownArrow);
            IM_CHECK_EQ(stb.cursor, char_count_per_line);
            SetCursorPosition(0); ctx->KeyPressMap(ImGuiKey_RightArrow);
            IM_CHECK_EQ(stb.cursor, 1);

            // End of first line
            SetCursorPosition(cursor_pos_end_of_first_line); ctx->KeyPressMap(ImGuiKey_UpArrow);
            IM_CHECK_EQ(stb.cursor, cursor_pos_end_of_first_line);
            SetCursorPosition(cursor_pos_end_of_first_line); ctx->KeyPressMap(ImGuiKey_LeftArrow);
            IM_CHECK_EQ(stb.cursor, cursor_pos_end_of_first_line - 1);
            SetCursorPosition(cursor_pos_end_of_first_line); ctx->KeyPressMap(ImGuiKey_DownArrow);
            IM_CHECK_EQ(stb.cursor, cursor_pos_end_of_first_line + char_count_per_line);
            SetCursorPosition(cursor_pos_end_of_first_line); ctx->KeyPressMap(ImGuiKey_RightArrow);
            IM_CHECK_EQ(stb.cursor, cursor_pos_end_of_first_line + 1);

            // Begin of last line
            SetCursorPosition(cursor_pos_begin_of_last_line); ctx->KeyPressMap(ImGuiKey_UpArrow);
            IM_CHECK_EQ(stb.cursor, cursor_pos_begin_of_last_line - char_count_per_line);
            SetCursorPosition(cursor_pos_begin_of_last_line); ctx->KeyPressMap(ImGuiKey_LeftArrow);
            IM_CHECK_EQ(stb.cursor, cursor_pos_begin_of_last_line - 1);
            SetCursorPosition(cursor_pos_begin_of_last_line); ctx->KeyPressMap(ImGuiKey_DownArrow);
            IM_CHECK_EQ(stb.cursor, has_trailing_line_feed ? eof : cursor_pos_begin_of_last_line);
            SetCursorPosition(cursor_pos_begin_of_last_line); ctx->KeyPressMap(ImGuiKey_RightArrow);
            IM_CHECK_EQ(stb.cursor, cursor_pos_begin_of_last_line + 1);

            // End of last line
            SetCursorPosition(cursor_pos_end_of_last_line); ctx->KeyPressMap(ImGuiKey_UpArrow);
            //IM_CHECK_EQ(stb.cursor, cursor_pos_end_of_last_line - char_count_per_line); // FIXME: This one is broken even on master
            SetCursorPosition(cursor_pos_end_of_last_line); ctx->KeyPressMap(ImGuiKey_LeftArrow);
            IM_CHECK_EQ(stb.cursor, cursor_pos_end_of_last_line - 1);
            SetCursorPosition(cursor_pos_end_of_last_line); ctx->KeyPressMap(ImGuiKey_DownArrow);
            IM_CHECK_EQ(stb.cursor, has_trailing_line_feed ? eof : cursor_pos_end_of_last_line);
            SetCursorPosition(cursor_pos_end_of_last_line); ctx->KeyPressMap(ImGuiKey_RightArrow);
            IM_CHECK_EQ(stb.cursor, cursor_pos_end_of_last_line + (has_trailing_line_feed ? 1 : 0));

            // In the middle of the content
            SetCursorPosition(cursor_pos_middle); ctx->KeyPressMap(ImGuiKey_UpArrow);
            IM_CHECK_EQ(stb.cursor, cursor_pos_middle - char_count_per_line);
            SetCursorPosition(cursor_pos_middle); ctx->KeyPressMap(ImGuiKey_LeftArrow);
            IM_CHECK_EQ(stb.cursor, cursor_pos_middle - 1);
            SetCursorPosition(cursor_pos_middle); ctx->KeyPressMap(ImGuiKey_DownArrow);
            IM_CHECK_EQ(stb.cursor, cursor_pos_middle + char_count_per_line);
            SetCursorPosition(cursor_pos_middle); ctx->KeyPressMap(ImGuiKey_RightArrow);
            IM_CHECK_EQ(stb.cursor, cursor_pos_middle + 1);

            // Home/End to go to beginning/end of the line
            SetCursorPosition(cursor_pos_middle); ctx->KeyPressMap(ImGuiKey_Home);
            IM_CHECK_EQ(stb.cursor, ((vars.LineCount / 2) - 1) * char_count_per_line);
            SetCursorPosition(cursor_pos_middle); ctx->KeyPressMap(ImGuiKey_End);
            IM_CHECK_EQ(stb.cursor, (vars.LineCount / 2) * char_count_per_line - 1);

            // Ctrl+Home/End to go to beginning/end of the text
            SetCursorPosition(cursor_pos_middle); ctx->KeyPressMap(ImGuiKey_Home, ImGuiKeyModFlags_Ctrl);
            IM_CHECK_EQ(stb.cursor, 0);
            SetCursorPosition(cursor_pos_middle); ctx->KeyPressMap(ImGuiKey_End, ImGuiKeyModFlags_Ctrl);
            IM_CHECK_EQ(stb.cursor, cursor_pos_end_of_last_line + (has_trailing_line_feed ? 1 : 0));

            // PageUp/PageDown
            SetCursorPosition(cursor_pos_begin_of_first_line); ctx->KeyPressMap(ImGuiKey_PageDown);
            IM_CHECK_EQ(stb.cursor, cursor_pos_begin_of_first_line + char_count_per_line * page_size);
            ctx->KeyPressMap(ImGuiKey_PageUp);
            IM_CHECK_EQ(stb.cursor, cursor_pos_begin_of_first_line);

            SetCursorPosition(cursor_pos_middle_of_first_line);
            ctx->KeyPressMap(ImGuiKey_PageDown);
            IM_CHECK_EQ(stb.cursor, cursor_pos_middle_of_first_line + char_count_per_line * page_size);
            ctx->KeyPressMap(ImGuiKey_PageDown);
            IM_CHECK_EQ(stb.cursor, cursor_pos_middle_of_first_line + char_count_per_line * page_size * 2);
            ctx->KeyPressMap(ImGuiKey_PageDown);
            IM_CHECK_EQ(stb.cursor, has_trailing_line_feed ? eof : eof - (char_count_per_line / 2) + 1);

            // We started PageDown from the middle of a line, so even if we're at the end (with X = 0),
            // PageUp should bring us one page up to the middle of the line
            int cursor_pos_begin_current_line = (stb.cursor / char_count_per_line) * char_count_per_line; // Round up cursor position to decimal only
            ctx->KeyPressMap(ImGuiKey_PageUp);
            IM_CHECK_EQ(stb.cursor, cursor_pos_begin_current_line - (page_size * char_count_per_line) + (char_count_per_line / 2));
                //eof - (char_count_per_line * page_size) + (char_count_per_line / 2) + (has_trailing_line_feed ? 0 : 1));
        }

        // Cursor positioning after new line. Broken line indexing may produce incorrect results in such case.
        ctx->KeyCharsReplaceEnter("foo");
        IM_CHECK_EQ(stb.cursor, 4);

        // Verify that cursor placement does not leak spaces in password field. (#4155)
        ctx->ItemClick("Password");
        ctx->KeyCharsAppendEnter("Totally not Password123");
        ctx->ItemDoubleClick("Password");
        IM_CHECK_EQ(stb.select_start, 0);
        IM_CHECK_EQ(stb.select_end, 23);
        IM_CHECK_EQ(stb.cursor, 23);
        ctx->KeyPressMap(ImGuiKey_LeftArrow, ImGuiKeyModFlags_Shortcut);
        IM_CHECK_EQ(stb.cursor, 0);
        ctx->KeyPressMap(ImGuiKey_RightArrow, ImGuiKeyModFlags_Shortcut);
        IM_CHECK_EQ(stb.cursor, 23);
    };
    // ## Test input text multiline cursor with selection: left, up, right, down, origin, end, ctrl+origin, ctrl+end, page up, page down
    // ## Test input text multiline scroll movement only: ctrl + (left, up, right, down)
    // ## Test input text multiline page up/page down history ?

    t = IM_REGISTER_TEST(e, "widgets", "widgets_inputtext_filters");
    struct InputTextFilterVars { Str64 Default; Str64 Decimal; Str64 Scientific;  Str64 Hex; Str64 Uppercase; Str64 NoBlank; Str64 Custom; };
    t->SetUserDataType<InputTextFilterVars>();
    t->GuiFunc = [](ImGuiTestContext* ctx)
    {
        InputTextFilterVars& vars = ctx->GetUserData<InputTextFilterVars>();
        struct TextFilters
        {
            // Return 0 (pass) if the character is 'i' or 'm' or 'g' or 'u' or 'i'
            static int FilterImGuiLetters(ImGuiInputTextCallbackData* data)
            {
                if (data->EventChar < 256 && strchr("imgui", (char)data->EventChar))
                    return 0;
                return 1;
            }
        };

        ImGui::Begin("Test Window", NULL, ImGuiWindowFlags_NoSavedSettings);
        ImGui::InputText("default", &vars.Default);
        ImGui::InputText("decimal", &vars.Decimal, ImGuiInputTextFlags_CharsDecimal);
        ImGui::InputText("scientific", &vars.Scientific, ImGuiInputTextFlags_CharsScientific);
        ImGui::InputText("hexadecimal", &vars.Hex, ImGuiInputTextFlags_CharsHexadecimal | ImGuiInputTextFlags_CharsUppercase);
        ImGui::InputText("uppercase", &vars.Uppercase, ImGuiInputTextFlags_CharsUppercase);
        ImGui::InputText("no blank", &vars.NoBlank, ImGuiInputTextFlags_CharsNoBlank);
        ImGui::InputText("\"imgui\" letters", &vars.Custom, ImGuiInputTextFlags_CallbackCharFilter, TextFilters::FilterImGuiLetters);
        ImGui::End();
    };
    t->TestFunc = [](ImGuiTestContext* ctx)
    {
        InputTextFilterVars& vars = ctx->GetUserData<InputTextFilterVars>();
        const char* input_text = "Some fancy Input Text in 0.., 1.., 2.., 3!";
        ctx->SetRef("Test Window");
        ctx->ItemClick("default");
        ctx->KeyCharsAppendEnter(input_text);
        IM_CHECK_STR_EQ(vars.Default.c_str(), input_text);

        ctx->ItemClick("decimal");
        ctx->KeyCharsAppendEnter(input_text);
        IM_CHECK_STR_EQ(vars.Decimal.c_str(), "0..1..2..3");

        ctx->ItemClick("scientific");
        ctx->KeyCharsAppendEnter(input_text);
        IM_CHECK_STR_EQ(vars.Scientific.c_str(), "ee0..1..2..3");

        ctx->ItemClick("hexadecimal");
        ctx->KeyCharsAppendEnter(input_text);
        IM_CHECK_STR_EQ(vars.Hex.c_str(), "EFACE0123");

        ctx->ItemClick("uppercase");
        ctx->KeyCharsAppendEnter(input_text);
        IM_CHECK_STR_EQ(vars.Uppercase.c_str(), "SOME FANCY INPUT TEXT IN 0.., 1.., 2.., 3!");

        ctx->ItemClick("no blank");
        ctx->KeyCharsAppendEnter(input_text);
        IM_CHECK_STR_EQ(vars.NoBlank.c_str(), "SomefancyInputTextin0..,1..,2..,3!");

        ctx->ItemClick("\"imgui\" letters");
        ctx->KeyCharsAppendEnter(input_text);
        IM_CHECK_STR_EQ(vars.Custom.c_str(), "mui");
    };

    // ## Test completion and history
    t = IM_REGISTER_TEST(e, "widgets", "widgets_inputtext_callback_misc");
    struct InputTextCallbackHistoryVars { Str CompletionBuffer; Str HistoryBuffer; Str EditBuffer; int EditCount = 0; };
    t->SetUserDataType<InputTextCallbackHistoryVars>();
    t->GuiFunc = [](ImGuiTestContext* ctx)
    {
        struct Funcs
        {
            static int MyCallback(ImGuiInputTextCallbackData* data)
            {
                if (data->EventFlag == ImGuiInputTextFlags_CallbackCompletion)
                {
                    data->InsertChars(data->CursorPos, "..");
                }
                else if (data->EventFlag == ImGuiInputTextFlags_CallbackHistory)
                {
                    if (data->EventKey == ImGuiKey_UpArrow)
                    {
                        data->DeleteChars(0, data->BufTextLen);
                        data->InsertChars(0, "Pressed Up!");
                        data->SelectAll();
                    }
                    else if (data->EventKey == ImGuiKey_DownArrow)
                    {
                        data->DeleteChars(0, data->BufTextLen);
                        data->InsertChars(0, "Pressed Down!");
                        data->SelectAll();
                    }
                }
                else if (data->EventFlag == ImGuiInputTextFlags_CallbackEdit)
                {
                    // Toggle casing of first character
                    char c = data->Buf[0];
                    if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')) data->Buf[0] ^= 32;
                    data->BufDirty = true;

                    // Increment a counter
                    int* p_edit_count = (int*)data->UserData;
                    *p_edit_count = *p_edit_count + 1;
                }
                return 0;
            }
        };

        InputTextCallbackHistoryVars& vars = ctx->GetUserData<InputTextCallbackHistoryVars>();
        ImGui::Begin("Test Window", NULL, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_AlwaysAutoResize);
        ImGui::InputText("Completion", &vars.CompletionBuffer, ImGuiInputTextFlags_CallbackCompletion, Funcs::MyCallback);
        ImGui::InputText("History", &vars.HistoryBuffer, ImGuiInputTextFlags_CallbackHistory, Funcs::MyCallback);
        ImGui::InputText("Edit", &vars.EditBuffer, ImGuiInputTextFlags_CallbackEdit, Funcs::MyCallback, (void*)&vars.EditCount);
        ImGui::SameLine(); ImGui::Text("(%d)", vars.EditCount);
        ImGui::End();
    };
    t->TestFunc = [](ImGuiTestContext* ctx)
    {
        InputTextCallbackHistoryVars& vars = ctx->GetUserData<InputTextCallbackHistoryVars>();

        ctx->SetRef("Test Window");
        ctx->ItemClick("Completion");
        ctx->KeyCharsAppend("Hello World");
        IM_CHECK_STR_EQ(vars.CompletionBuffer.c_str(), "Hello World");
        ctx->KeyPressMap(ImGuiKey_Tab);
        IM_CHECK_STR_EQ(vars.CompletionBuffer.c_str(), "Hello World..");

        // FIXME: Not testing History callback :)
        ctx->ItemClick("History");
        ctx->KeyCharsAppend("ABCDEF");
        ctx->KeyPressMap(ImGuiKey_Z, ImGuiKeyModFlags_Shortcut);
        IM_CHECK_STR_EQ(vars.HistoryBuffer.c_str(), "ABCDE");
        ctx->KeyPressMap(ImGuiKey_Z, ImGuiKeyModFlags_Shortcut);
        ctx->KeyPressMap(ImGuiKey_Z, ImGuiKeyModFlags_Shortcut);
        IM_CHECK_STR_EQ(vars.HistoryBuffer.c_str(), "ABC");
        ctx->KeyPressMap(ImGuiKey_Y, ImGuiKeyModFlags_Shortcut);
        IM_CHECK_STR_EQ(vars.HistoryBuffer.c_str(), "ABCD");
        ctx->KeyPressMap(ImGuiKey_UpArrow);
        IM_CHECK_STR_EQ(vars.HistoryBuffer.c_str(), "Pressed Up!");
        ctx->KeyPressMap(ImGuiKey_DownArrow);
        IM_CHECK_STR_EQ(vars.HistoryBuffer.c_str(), "Pressed Down!");

        ctx->ItemClick("Edit");
        IM_CHECK_STR_EQ(vars.EditBuffer.c_str(), "");
        IM_CHECK_EQ(vars.EditCount, 0);
        ctx->KeyCharsAppend("h");
        IM_CHECK_STR_EQ(vars.EditBuffer.c_str(), "H");
        IM_CHECK_EQ(vars.EditCount, 1);
        ctx->KeyCharsAppend("e");
        IM_CHECK_STR_EQ(vars.EditBuffer.c_str(), "he");
        IM_CHECK_EQ(vars.EditCount, 2);
        ctx->KeyCharsAppend("llo");
        IM_CHECK_STR_EQ(vars.EditBuffer.c_str(), "Hello");
        IM_CHECK_LE(vars.EditCount, ctx->EngineIO->ConfigRunFast ? 3 : 5); // If running fast, "llo" will be considered as one edit only
    };

    // ## Test character replacement in callback (inspired by https://github.com/ocornut/imgui/pull/3587)
    t = IM_REGISTER_TEST(e, "widgets", "widgets_inputtext_callback_replace");
    t->GuiFunc = [](ImGuiTestContext* ctx)
    {
        ImGuiTestGenericVars& vars = ctx->GenericVars;
        ImGui::Begin("Test Window", NULL, ImGuiWindowFlags_NoSavedSettings);
        auto callback = [](ImGuiInputTextCallbackData* data)
        {
            if (data->CursorPos >= 3 && strcmp(data->Buf + data->CursorPos - 3, "abc") == 0)
            {
                data->DeleteChars(data->CursorPos - 3, 3);
                data->InsertChars(data->CursorPos, "\xE5\xA5\xBD"); // HAO
                data->SelectionStart = data->CursorPos - 3;
                data->SelectionEnd = data->CursorPos;
                return 1;
            }
            return 0;
        };
        ImGui::InputText("Hello", vars.Str1, IM_ARRAYSIZE(vars.Str1), ImGuiInputTextFlags_CallbackAlways, callback);
        ImGui::End();
    };
    t->TestFunc = [](ImGuiTestContext* ctx)
    {
        ctx->SetRef("Test Window");
        ctx->ItemInput("Hello");
        ImGuiInputTextState* state = &ctx->UiContext->InputTextState;
        IM_CHECK(state && state->ID == ctx->GetID("Hello"));
        ctx->KeyCharsAppend("ab");
        IM_CHECK(state->CurLenA == 2);
        IM_CHECK(state->CurLenW == 2);
        IM_CHECK(strcmp(state->TextA.Data, "ab") == 0);
        IM_CHECK(state->Stb.cursor == 2);
        ctx->KeyCharsAppend("c");
        IM_CHECK(state->CurLenA == 3);
        IM_CHECK(state->CurLenW == 1);
        IM_CHECK(strcmp(state->TextA.Data, "\xE5\xA5\xBD") == 0);
        IM_CHECK(state->TextW.Data[0] == 0x597D);
        IM_CHECK(state->TextW.Data[1] == 0);
        IM_CHECK(state->Stb.cursor == 1);
        IM_CHECK(state->Stb.select_start == 0 && state->Stb.select_end == 1);
    };

    // ## Test resize callback (#3009, #2006, #1443, #1008)
    t = IM_REGISTER_TEST(e, "widgets", "widgets_inputtext_callback_resize");
    struct StrVars { Str str; };
    t->SetUserDataType<StrVars>();
    t->GuiFunc = [](ImGuiTestContext* ctx)
    {
        StrVars& vars = ctx->GetUserData<StrVars>();
        ImGui::Begin("Test Window", NULL, ImGuiWindowFlags_NoSavedSettings);
        if (ImGui::InputText("Field1", &vars.str, ImGuiInputTextFlags_EnterReturnsTrue))
        {
            IM_CHECK_EQ(vars.str.capacity(), 4 + 5 + 1);
            IM_CHECK_STR_EQ(vars.str.c_str(), "abcdhello");
        }
        Str str_local_unsaved = "abcd";
        if (ImGui::InputText("Field2", &str_local_unsaved, ImGuiInputTextFlags_EnterReturnsTrue))
        {
            IM_CHECK_EQ(str_local_unsaved.capacity(), 4 + 5 + 1);
            IM_CHECK_STR_EQ(str_local_unsaved.c_str(), "abcdhello");
        }
        ImGui::End();

    };
    t->TestFunc = [](ImGuiTestContext* ctx)
    {
        StrVars& vars = ctx->GetUserData<StrVars>();
        vars.str.set("abcd");
        IM_CHECK_EQ(vars.str.capacity(), 4 + 1);
        ctx->SetRef("Test Window");
        ctx->ItemInput("Field1");
        ctx->KeyCharsAppendEnter("hello");
        ctx->ItemInput("Field2");
        ctx->KeyCharsAppendEnter("hello");
    };

    // ## Test for Nav interference
    t = IM_REGISTER_TEST(e, "widgets", "widgets_inputtext_nav");
    t->GuiFunc = [](ImGuiTestContext* ctx)
    {
        ImGuiTestGenericVars& vars = ctx->GenericVars;
        ImGui::Begin("Test Window", NULL, ImGuiWindowFlags_NoSavedSettings);
        ImVec2 sz(50, 0);
        ImGui::Button("UL", sz); ImGui::SameLine();
        ImGui::Button("U",  sz); ImGui::SameLine();
        ImGui::Button("UR", sz);
        ImGui::Button("L",  sz); ImGui::SameLine();
        ImGui::SetNextItemWidth(sz.x);
        ImGui::InputText("##Field", vars.Str1, IM_ARRAYSIZE(vars.Str1), ImGuiInputTextFlags_AllowTabInput);
        ImGui::SameLine();
        ImGui::Button("R", sz);
        ImGui::Button("DL", sz); ImGui::SameLine();
        ImGui::Button("D", sz); ImGui::SameLine();
        ImGui::Button("DR", sz);
        ImGui::End();
    };
    t->TestFunc = [](ImGuiTestContext* ctx)
    {
        ctx->SetRef("Test Window");
        ctx->ItemClick("##Field");
        ctx->KeyPressMap(ImGuiKey_LeftArrow);
        IM_CHECK_EQ(ctx->UiContext->NavId, ctx->GetID("##Field"));
        ctx->KeyPressMap(ImGuiKey_RightArrow);
        IM_CHECK_EQ(ctx->UiContext->NavId, ctx->GetID("##Field"));
        ctx->KeyPressMap(ImGuiKey_UpArrow);
        IM_CHECK_EQ(ctx->UiContext->NavId, ctx->GetID("U"));
        ctx->KeyPressMap(ImGuiKey_DownArrow);
        ctx->KeyPressMap(ImGuiKey_DownArrow);
        IM_CHECK_EQ(ctx->UiContext->NavId, ctx->GetID("D"));
    };

    // ## Test InputText widget with user a buffer on stack, reading/writing past end of buffer.
    t = IM_REGISTER_TEST(e, "widgets", "widgets_inputtext_temp_buffer");
    t->GuiFunc = [](ImGuiTestContext* ctx)
    {
        ImGui::Begin("Test Window", NULL, ImGuiWindowFlags_NoSavedSettings);
        char buf[10] = "foo\0xxxxx";
        if (ImGui::InputText("Field", buf, 7, ImGuiInputTextFlags_EnterReturnsTrue))
        {
            IM_CHECK_STR_EQ(buf, "foobar");
            IM_CHECK_STR_EQ(buf + 7, "xx"); // Buffer padding not overwritten
        }
        ImGui::End();
    };
    t->TestFunc = [](ImGuiTestContext* ctx)
    {
        ctx->SetRef("Test Window");
        ctx->ItemClick("Field");
        ctx->KeyCharsAppendEnter("barrr");
    };

    // ## Test inheritance of ItemFlags
    t = IM_REGISTER_TEST(e, "widgets", "widgets_item_flags_stack");
    t->GuiFunc = [](ImGuiTestContext* ctx)
    {
        ImGuiContext& g = *ctx->UiContext;
        ImGui::Begin("Test Window", NULL, ImGuiWindowFlags_NoSavedSettings);
        IM_CHECK_EQ(g.CurrentItemFlags, 0);

        ImGui::BeginChild("child1", ImVec2(100, 100));
        IM_CHECK_EQ(g.CurrentItemFlags, 0);
        ImGui::Button("enable button in child1");
        ImGui::EndChild();

        ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
        ImGui::Button("disabled button in parent");

        IM_CHECK_EQ(g.CurrentItemFlags, ImGuiItemFlags_Disabled);
        ImGui::BeginChild("child1"); // Append
        IM_CHECK_EQ(g.CurrentItemFlags, ImGuiItemFlags_Disabled);
        ImGui::Button("disabled button in child1");
        ImGui::EndChild();

        ImGui::BeginChild("child2", ImVec2(100, 100)); // New
        IM_CHECK_EQ(g.CurrentItemFlags, ImGuiItemFlags_Disabled);
        ImGui::Button("disabled button in child2");
        ImGui::EndChild();

        ImGui::PopItemFlag();

#if IMGUI_VERSION_NUM >= 18207
        IM_CHECK_EQ(g.CurrentItemFlags, 0);
        ImGui::Begin("Test Window 2", NULL, ImGuiWindowFlags_NoSavedSettings);
        ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
        ImGui::End();
        IM_CHECK_EQ(g.CurrentItemFlags, ImGuiItemFlags_Disabled);
        ImGui::PopItemFlag();
        IM_CHECK_EQ(g.CurrentItemFlags, 0);
#endif

        ImGui::End();
    };

    // ## Test ColorEdit4() and IsItemDeactivatedXXX() functions
    // ## Test that IsItemActivated() doesn't trigger when clicking the color button to open picker
    t = IM_REGISTER_TEST(e, "widgets", "widgets_status_coloredit");
    t->GuiFunc = [](ImGuiTestContext* ctx)
    {
        ImGuiTestGenericVars& vars = ctx->GenericVars;
        ImGui::Begin("Test Window", NULL, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_AlwaysAutoResize);
        bool ret = ImGui::ColorEdit4("Field", &vars.Vec4.x, ImGuiColorEditFlags_None);
        vars.Status.QueryInc(ret);
        ImGui::End();
    };
    t->TestFunc = [](ImGuiTestContext* ctx)
    {
        // Accumulate return values over several frames/action into each bool
        ImGuiTestGenericVars& vars = ctx->GenericVars;
        ImGuiTestGenericStatus& status = vars.Status;

        // Testing activation flag being set
        ctx->SetRef("Test Window");
        ctx->ItemClick("Field/##ColorButton");
        IM_CHECK(status.Ret == 0 && status.Activated == 1 && status.Deactivated == 1 && status.DeactivatedAfterEdit == 0 && status.Edited == 0);
        status.Clear();

        ctx->KeyPressMap(ImGuiKey_Escape);
        IM_CHECK(status.Ret == 0 && status.Activated == 0 && status.Deactivated == 0 && status.DeactivatedAfterEdit == 0 && status.Edited == 0);
        status.Clear();
    };

    // ## Test InputText() and IsItemDeactivatedXXX() functions (mentioned in #2215)
    t = IM_REGISTER_TEST(e, "widgets", "widgets_status_inputtext");
    t->GuiFunc = [](ImGuiTestContext* ctx)
    {
        ImGuiTestGenericVars& vars = ctx->GenericVars;
        ImGui::Begin("Test Window", NULL, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_AlwaysAutoResize);
        bool ret = ImGui::InputText("Field", vars.Str1, IM_ARRAYSIZE(vars.Str1));
        vars.Status.QueryInc(ret);
        ImGui::InputText("Sibling", vars.Str2, IM_ARRAYSIZE(vars.Str2));
        ImGui::End();
    };
    t->TestFunc = [](ImGuiTestContext* ctx)
    {
        // Accumulate return values over several frames/action into each bool
        ImGuiTestGenericVars& vars = ctx->GenericVars;
        ImGuiTestGenericStatus& status = vars.Status;

        // Testing activation flag being set
        ctx->SetRef("Test Window");
        ctx->ItemClick("Field");
        IM_CHECK(status.Ret == 0 && status.Activated == 1 && status.Deactivated == 0 && status.DeactivatedAfterEdit == 0 && status.Edited == 0);
        status.Clear();

        // Testing deactivated flag being set when canceling with Escape
        ctx->KeyPressMap(ImGuiKey_Escape);
        IM_CHECK(status.Ret == 0 && status.Activated == 0 && status.Deactivated == 1 && status.DeactivatedAfterEdit == 0 && status.Edited == 0);
        status.Clear();

        // Testing validation with Return after editing
        ctx->ItemClick("Field");
        IM_CHECK(!status.Ret && status.Activated && !status.Deactivated && !status.DeactivatedAfterEdit && status.Edited == 0);
        status.Clear();
        ctx->KeyCharsAppend("Hello");
        IM_CHECK(status.Ret && !status.Activated && !status.Deactivated && !status.DeactivatedAfterEdit && status.Edited >= 1);
        status.Clear();
        ctx->KeyPressMap(ImGuiKey_Enter);
        IM_CHECK(!status.Ret && !status.Activated && status.Deactivated && status.DeactivatedAfterEdit && status.Edited == 0);
        status.Clear();

        // Testing validation with Tab after editing
        ctx->ItemClick("Field");
        ctx->KeyCharsAppend(" World");
        IM_CHECK(status.Ret && status.Activated && !status.Deactivated && !status.DeactivatedAfterEdit && status.Edited >= 1);
        status.Clear();
        ctx->KeyPressMap(ImGuiKey_Tab);
        IM_CHECK(!status.Ret && !status.Activated && status.Deactivated && status.DeactivatedAfterEdit && status.Edited == 0);
        status.Clear();
    };

    // ## Test the IsItemDeactivatedXXX() functions (e.g. #2550, #1875)
    t = IM_REGISTER_TEST(e, "widgets", "widgets_status_multicomponent");
    t->GuiFunc = [](ImGuiTestContext* ctx)
    {
        ImGuiTestGenericVars& vars = ctx->GenericVars;
        ImGui::Begin("Test Window", NULL, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_AlwaysAutoResize);
        bool ret = ImGui::InputFloat4("Field", &vars.FloatArray[0]);
        vars.Status.QueryInc(ret);
        ImGui::End();
    };
    t->TestFunc = [](ImGuiTestContext* ctx)
    {
        // Accumulate return values over several frames/action into each bool
        ImGuiTestGenericVars& vars = ctx->GenericVars;
        ImGuiTestGenericStatus& status = vars.Status;

        // FIXME-TESTS: Better helper to build ids out of various type of data
        ctx->SetRef("Test Window");
        int n;
        n = 0; ImGuiID field_0 = ImHashData(&n, sizeof(n), ctx->GetID("Field"));
        n = 1; ImGuiID field_1 = ImHashData(&n, sizeof(n), ctx->GetID("Field"));
        //n = 2; ImGuiID field_2 = ImHashData(&n, sizeof(n), ctx->GetID("Field"));

        // Testing activation/deactivation flags
        ctx->ItemClick(field_0);
        IM_CHECK(status.Ret == 0 && status.Activated == 1 && status.Deactivated == 0 && status.DeactivatedAfterEdit == 0);
        status.Clear();
        ctx->KeyPressMap(ImGuiKey_Enter);
        IM_CHECK(status.Ret == 0 && status.Activated == 0 && status.Deactivated == 1 && status.DeactivatedAfterEdit == 0);
        status.Clear();

        // Testing validation with Return after editing
        ctx->ItemClick(field_0);
        status.Clear();
        ctx->KeyCharsAppend("123");
        IM_CHECK(status.Ret >= 1 && status.Activated == 0 && status.Deactivated == 0);
        status.Clear();
        ctx->KeyPressMap(ImGuiKey_Enter);
        IM_CHECK(status.Ret == 0 && status.Activated == 0 && status.Deactivated == 1);
        status.Clear();

        // Testing validation with Tab after editing
        ctx->ItemClick(field_0);
        ctx->KeyCharsAppend("456");
        status.Clear();
        ctx->KeyPressMap(ImGuiKey_Tab);
        IM_CHECK(status.Ret == 0 && status.Activated == 1 && status.Deactivated == 1 && status.DeactivatedAfterEdit == 1);

        // Testing Edited flag on all components
        ctx->ItemClick(field_1); // FIXME-TESTS: Should not be necessary!
        ctx->ItemClick(field_0);
        ctx->KeyCharsAppend("111");
        IM_CHECK(status.Edited >= 1);
        ctx->KeyPressMap(ImGuiKey_Tab);
        status.Clear();
        ctx->KeyCharsAppend("222");
        IM_CHECK(status.Edited >= 1);
        ctx->KeyPressMap(ImGuiKey_Tab);
        status.Clear();
        ctx->KeyCharsAppend("333");
        IM_CHECK(status.Edited >= 1);
    };

    // ## Test the IsItemEdited() function when input vs output format are not matching
    t = IM_REGISTER_TEST(e, "widgets", "widgets_status_inputfloat_format_mismatch");
    t->GuiFunc = [](ImGuiTestContext* ctx)
    {
        ImGuiTestGenericVars& vars = ctx->GenericVars;
        ImGui::Begin("Test Window", NULL, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_AlwaysAutoResize);
        bool ret = ImGui::InputFloat("Field", &vars.Float1);
        vars.Status.QueryInc(ret);
        ImGui::End();
    };
    t->TestFunc = [](ImGuiTestContext* ctx)
    {
        ImGuiTestGenericVars& vars = ctx->GenericVars;
        ImGuiTestGenericStatus& status = vars.Status;

        // Input "1" which will be formatted as "1.000", make sure we don't report IsItemEdited() multiple times!
        ctx->SetRef("Test Window");
        ctx->ItemClick("Field");
        ctx->KeyCharsAppend("1");
        IM_CHECK(status.Ret == 1 && status.Edited == 1 && status.Activated == 1 && status.Deactivated == 0 && status.DeactivatedAfterEdit == 0);
        ctx->Yield();
        ctx->Yield();
        IM_CHECK(status.Edited == 1);
    };

    // ## Test that tight tab bar does not create extra drawcalls
    t = IM_REGISTER_TEST(e, "widgets", "widgets_tabbar_drawcalls");
    t->GuiFunc = [](ImGuiTestContext* ctx)
    {
        ImGui::Begin("Test Window", NULL, ImGuiWindowFlags_NoSavedSettings);
        if (ImGui::BeginTabBar("Tab Drawcalls"))
        {
            for (int i = 0; i < 20; i++)
                if (ImGui::BeginTabItem(Str30f("Tab %d", i).c_str()))
                    ImGui::EndTabItem();
            ImGui::EndTabBar();
        }
        ImGui::End();
    };
    t->TestFunc = [](ImGuiTestContext* ctx)
    {
        ImGuiWindow* window = ImGui::FindWindowByName("Test Window");
        ctx->WindowResize("Test Window", ImVec2(300, 300));
        int draw_calls = window->DrawList->CmdBuffer.Size;
        ctx->WindowResize("Test Window", ImVec2(1, 1));
        IM_CHECK(draw_calls == window->DrawList->CmdBuffer.Size);
    };

    // ## Test order of tabs in a tab bar
    t = IM_REGISTER_TEST(e, "widgets", "widgets_tabbar_order");
    t->GuiFunc = [](ImGuiTestContext* ctx)
    {
        auto& vars = ctx->GenericVars;
        ImGui::Begin("Test Window", NULL, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_AlwaysAutoResize);
        for (int n = 0; n < 4; n++)
            ImGui::Checkbox(Str30f("Open Tab %d", n).c_str(), &vars.BoolArray[n]);
        if (ImGui::BeginTabBar("TabBar", ImGuiTabBarFlags_Reorderable))
        {
            for (int n = 0; n < 4; n++)
                if (vars.BoolArray[n] && ImGui::BeginTabItem(Str30f("Tab %d", n).c_str(), &vars.BoolArray[n]))
                    ImGui::EndTabItem();
            ImGui::EndTabBar();
        }
        ImGui::End();
    };
    t->TestFunc = [](ImGuiTestContext* ctx)
    {
        ImGuiContext& g = *ctx->UiContext;
        auto& vars = ctx->GenericVars;
        ctx->SetRef("Test Window");
        ImGuiTabBar* tab_bar = g.TabBars.GetOrAddByKey(ctx->GetID("TabBar")); // FIXME-TESTS: Helper function?
        IM_CHECK(tab_bar != NULL);
        IM_CHECK(tab_bar->Tabs.Size == 0);

        vars.BoolArray[0] = vars.BoolArray[1] = vars.BoolArray[2] = true;
        ctx->Yield();
        ctx->Yield(); // Important: so tab layout are correct for TabClose()
        IM_CHECK(tab_bar->Tabs.Size == 3);
        IM_CHECK_STR_EQ(tab_bar->GetTabName(&tab_bar->Tabs[0]), "Tab 0");
        IM_CHECK_STR_EQ(tab_bar->GetTabName(&tab_bar->Tabs[1]), "Tab 1");
        IM_CHECK_STR_EQ(tab_bar->GetTabName(&tab_bar->Tabs[2]), "Tab 2");

        ctx->TabClose("TabBar/Tab 1");
        ctx->Yield();
        ctx->Yield();
        IM_CHECK(vars.BoolArray[1] == false);
        IM_CHECK(tab_bar->Tabs.Size == 2);
        IM_CHECK_STR_EQ(tab_bar->GetTabName(&tab_bar->Tabs[0]), "Tab 0");
        IM_CHECK_STR_EQ(tab_bar->GetTabName(&tab_bar->Tabs[1]), "Tab 2");

        vars.BoolArray[1] = true;
        ctx->Yield();
        IM_CHECK(tab_bar->Tabs.Size == 3);
        IM_CHECK_STR_EQ(tab_bar->GetTabName(&tab_bar->Tabs[0]), "Tab 0");
        IM_CHECK_STR_EQ(tab_bar->GetTabName(&tab_bar->Tabs[1]), "Tab 2");
        IM_CHECK_STR_EQ(tab_bar->GetTabName(&tab_bar->Tabs[2]), "Tab 1");
    };

    // ## (Attempt to) Test that tab bar declares its unclipped size.
    t = IM_REGISTER_TEST(e, "widgets", "widgets_tabbar_size");
    struct TabBarVars { bool HasCloseButton = false; float ExpectedWidth = 0.0f; };
    t->SetUserDataType<TabBarVars>();
    t->GuiFunc = [](ImGuiTestContext* ctx)
    {
        ImGuiContext& g = *ctx->UiContext;
        auto& vars = ctx->GetUserData<TabBarVars>();

        // FIXME-TESTS: Ideally we would test variation of with/without ImGuiTabBarFlags_TabListPopupButton, but we'd need to know its width...
        ImGui::Begin("Test Window", NULL, ImGuiWindowFlags_NoSavedSettings);
        ImGui::Checkbox("HasCloseButton", &vars.HasCloseButton);
        if (ImGui::BeginTabBar("TabBar"))
        {
            vars.ExpectedWidth = 0.0f;
            for (int i = 0; i < 3; i++)
            {
                Str30f label("TabItem %d", i);
                bool tab_open = true;
                if (ImGui::BeginTabItem(label.c_str(), vars.HasCloseButton ? &tab_open : NULL))
                    ImGui::EndTabItem();
                if (i > 0)
                    vars.ExpectedWidth += g.Style.ItemInnerSpacing.x;
                vars.ExpectedWidth += ImGui::TabItemCalcSize(label.c_str(), vars.HasCloseButton).x;
            }
            ImGui::EndTabBar();
        }
        ImGui::End();
    };
    t->TestFunc = [](ImGuiTestContext* ctx)
    {
        ImGuiWindow* window = ImGui::FindWindowByName("Test Window");
        auto& vars = ctx->GetUserData<TabBarVars>();

        vars.HasCloseButton = false;
        ctx->Yield();
        IM_CHECK_EQ(window->DC.CursorStartPos.x + vars.ExpectedWidth, window->DC.IdealMaxPos.x);

        vars.HasCloseButton = true;
        ctx->Yield(); // BeginTabBar() will submit old size --> TabBarLayout update sizes
        ctx->Yield(); // BeginTabBar() will submit new size
        IM_CHECK_EQ(window->DC.CursorStartPos.x + vars.ExpectedWidth, window->DC.IdealMaxPos.x);
    };

    // ## Test TabItemButton behavior
    t = IM_REGISTER_TEST(e, "widgets", "widgets_tabbar_tabitem_button");
    struct TabBarButtonVars { int LastClickedButton = -1; };
    t->SetUserDataType<TabBarButtonVars>();
    t->GuiFunc = [](ImGuiTestContext* ctx)
    {
        TabBarButtonVars& vars = ctx->GetUserData<TabBarButtonVars>();
        ImGui::Begin("Test Window", NULL, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings);
        if (ImGui::BeginTabBar("TabBar"))
        {
            if (ImGui::TabItemButton("1", ImGuiTabItemFlags_None))        { vars.LastClickedButton = 1; }
            if (ImGui::TabItemButton("0", ImGuiTabItemFlags_None))        { vars.LastClickedButton = 0; }
            if (ImGui::BeginTabItem("Tab", NULL, ImGuiTabItemFlags_None)) { ImGui::EndTabItem(); }
            ImGui::EndTabBar();
        }
        ImGui::End();
    };
    t->TestFunc = [](ImGuiTestContext* ctx)
    {
        TabBarButtonVars& vars = ctx->GetUserData<TabBarButtonVars>();
        ctx->SetRef("Test Window/TabBar");

        IM_CHECK_EQ(vars.LastClickedButton, -1);
        ctx->ItemClick("1");
        IM_CHECK_EQ(vars.LastClickedButton, 1);
        ctx->ItemClick("Tab");
        IM_CHECK_EQ(vars.LastClickedButton, 1);
        ctx->MouseMove("0");
        ctx->MouseDown();
        IM_CHECK_EQ(vars.LastClickedButton, 1);
        ctx->MouseUp();
        IM_CHECK_EQ(vars.LastClickedButton, 0);
    };

    // ## Test that tab items respects their Leading/Trailing position
    t = IM_REGISTER_TEST(e, "widgets", "widgets_tabbar_tabitem_leading_trailing");
    struct TabBarLeadingTrailingVars { bool WindowAutoResize = true; ImGuiTabBarFlags TabBarFlags = 0; ImGuiTabBar* TabBar = NULL; };
    t->SetUserDataType<TabBarLeadingTrailingVars>();
    t->GuiFunc = [](ImGuiTestContext* ctx)
    {
        ImGuiContext& g = *ctx->UiContext;
        TabBarLeadingTrailingVars& vars = ctx->GetUserData<TabBarLeadingTrailingVars>();
        ImGui::Begin("Test Window", NULL, (vars.WindowAutoResize ? ImGuiWindowFlags_AlwaysAutoResize : 0) | ImGuiWindowFlags_NoSavedSettings);
        ImGui::Checkbox("ImGuiWindowFlags_AlwaysAutoResize", &vars.WindowAutoResize);
        if (ImGui::BeginTabBar("TabBar", vars.TabBarFlags))
        {
            vars.TabBar = g.CurrentTabBar;
            if (ImGui::BeginTabItem("Trailing", NULL, ImGuiTabItemFlags_Trailing)) { ImGui::EndTabItem(); } // Intentionally submit Trailing tab early and Leading tabs at the end
            if (ImGui::BeginTabItem("Tab 0", NULL, ImGuiTabItemFlags_None))        { ImGui::EndTabItem(); }
            if (ImGui::BeginTabItem("Tab 1", NULL, ImGuiTabItemFlags_None))        { ImGui::EndTabItem(); }
            if (ImGui::BeginTabItem("Tab 2", NULL, ImGuiTabItemFlags_None))        { ImGui::EndTabItem(); }
            if (ImGui::BeginTabItem("Leading", NULL, ImGuiTabItemFlags_Leading))   { ImGui::EndTabItem(); }
            ImGui::EndTabBar();
        }
        ImGui::End();
    };
    t->TestFunc = [](ImGuiTestContext* ctx)
    {
        ImGuiContext& g = *ctx->UiContext;
        auto& vars = ctx->GetUserData<TabBarLeadingTrailingVars>();

        vars.TabBarFlags = ImGuiTabBarFlags_Reorderable | ImGuiTabBarFlags_FittingPolicyResizeDown;
        ctx->Yield();

        ctx->SetRef("Test Window/TabBar");
        const char* tabs[] = { "Leading", "Tab 0", "Tab 1", "Tab 2", "Trailing" };

        // Check that tabs relative order matches what we expect (which is not the same as submission order above)
        float offset_x = -FLT_MAX;
        for (int i = 0; i < IM_ARRAYSIZE(tabs); ++i)
        {
            ctx->MouseMove(tabs[i]);
            IM_CHECK_GT(g.IO.MousePos.x, offset_x);
            offset_x = g.IO.MousePos.x;
        }

        // Test that "Leading" cannot be reordered over "Tab 0" and vice-versa
        ctx->ItemDragAndDrop("Leading", "Tab 0");
        IM_CHECK_EQ(vars.TabBar->Tabs[0].ID, ctx->GetID("Leading"));
        IM_CHECK_EQ(vars.TabBar->Tabs[1].ID, ctx->GetID("Tab 0"));
        ctx->ItemDragAndDrop("Tab 0", "Leading");
        IM_CHECK_EQ(vars.TabBar->Tabs[0].ID, ctx->GetID("Leading"));
        IM_CHECK_EQ(vars.TabBar->Tabs[1].ID, ctx->GetID("Tab 0"));

        // Test that "Trailing" cannot be reordered over "Tab 2" and vice-versa
        ctx->ItemDragAndDrop("Trailing", "Tab 2");
        IM_CHECK_EQ(vars.TabBar->Tabs[4].ID, ctx->GetID("Trailing"));
        IM_CHECK_EQ(vars.TabBar->Tabs[3].ID, ctx->GetID("Tab 2"));
        ctx->ItemDragAndDrop("Tab 2", "Trailing");
        IM_CHECK_EQ(vars.TabBar->Tabs[4].ID, ctx->GetID("Trailing"));
        IM_CHECK_EQ(vars.TabBar->Tabs[3].ID, ctx->GetID("Tab 2"));

        // Resize down
        vars.WindowAutoResize = false;
        ImGuiWindow* window = ctx->GetWindowByRef("/Test Window");
        ctx->WindowResize("Test Window", ImVec2(window->Size.x * 0.3f, window->Size.y));
        for (int i = 0; i < 2; ++i)
        {
            vars.TabBarFlags = ImGuiTabBarFlags_Reorderable | (i == 0 ? ImGuiTabBarFlags_FittingPolicyResizeDown : ImGuiTabBarFlags_FittingPolicyScroll);
            ctx->Yield();
            IM_CHECK_GT(ctx->ItemInfo("Leading")->RectClipped.GetWidth(), 1.0f);
            IM_CHECK_EQ(ctx->ItemInfo("Tab 0")->RectClipped.GetWidth(), 0.0f);
            IM_CHECK_EQ(ctx->ItemInfo("Tab 1")->RectClipped.GetWidth(), 0.0f);
            IM_CHECK_EQ(ctx->ItemInfo("Tab 2")->RectClipped.GetWidth(), 0.0f);
            IM_CHECK_GT(ctx->ItemInfo("Trailing")->RectClipped.GetWidth(), 1.0f);
        }
    };

    // ## Test reordering tabs (and ImGuiTabItemFlags_NoReorder flag)
    t = IM_REGISTER_TEST(e, "widgets", "widgets_tabbar_reorder");
    struct TabBarReorderVars { ImGuiTabBarFlags Flags = ImGuiTabBarFlags_Reorderable; ImGuiTabBar* TabBar = NULL; };
    t->SetUserDataType<TabBarReorderVars>();
    t->GuiFunc = [](ImGuiTestContext* ctx)
    {
        ImGuiContext& g = *ctx->UiContext;
        auto& vars = ctx->GetUserData<TabBarReorderVars>();
        ImGui::Begin("Test Window", NULL, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings);
        if (ImGui::BeginTabBar("TabBar", vars.Flags))
        {
            vars.TabBar = g.CurrentTabBar;
            if (ImGui::BeginTabItem("Tab 0", NULL, ImGuiTabItemFlags_None))      { ImGui::EndTabItem(); }
            if (ImGui::BeginTabItem("Tab 1", NULL, ImGuiTabItemFlags_None))      { ImGui::EndTabItem(); }
            if (ImGui::BeginTabItem("Tab 2", NULL, ImGuiTabItemFlags_NoReorder)) { ImGui::EndTabItem(); }
            if (ImGui::BeginTabItem("Tab 3", NULL, ImGuiTabItemFlags_None))      { ImGui::EndTabItem(); }
            ImGui::EndTabBar();
        }
        ImGui::End();
    };
    t->TestFunc = [](ImGuiTestContext* ctx)
    {
        TabBarReorderVars& vars = ctx->GetUserData<TabBarReorderVars>();

        // Reset reorderable flags to ensure tabs are in their submission order
        vars.Flags = ImGuiTabBarFlags_None;
        ctx->Yield();
        vars.Flags = ImGuiTabBarFlags_Reorderable;
        ctx->Yield();

        ctx->SetRef("Test Window/TabBar");

        ctx->ItemDragAndDrop("Tab 0", "Tab 1");
        IM_CHECK_EQ(vars.TabBar->Tabs[0].ID, ctx->GetID("Tab 1"));
        IM_CHECK_EQ(vars.TabBar->Tabs[1].ID, ctx->GetID("Tab 0"));

        ctx->ItemDragAndDrop("Tab 0", "Tab 1");
        IM_CHECK_EQ(vars.TabBar->Tabs[0].ID, ctx->GetID("Tab 0"));
        IM_CHECK_EQ(vars.TabBar->Tabs[1].ID, ctx->GetID("Tab 1"));

        ctx->ItemDragAndDrop("Tab 0", "Tab 2"); // Tab 2 has NoReorder flag
        ctx->ItemDragAndDrop("Tab 0", "Tab 3"); // Tab 2 has NoReorder flag
        ctx->ItemDragAndDrop("Tab 3", "Tab 2"); // Tab 2 has NoReorder flag
        IM_CHECK_EQ(vars.TabBar->Tabs[1].ID, ctx->GetID("Tab 0"));
        IM_CHECK_EQ(vars.TabBar->Tabs[2].ID, ctx->GetID("Tab 2"));
        IM_CHECK_EQ(vars.TabBar->Tabs[3].ID, ctx->GetID("Tab 3"));
    };

    // ## Test nested/recursing Tab Bars (Bug #2371)
    t = IM_REGISTER_TEST(e, "widgets", "widgets_tabbar_nested");
    t->GuiFunc = [](ImGuiTestContext* ctx)
    {
        ImGui::Begin("Test Window", NULL, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_AlwaysAutoResize);
        if (ImGui::BeginTabBar("TabBar 0"))
        {
            if (ImGui::BeginTabItem("TabItem"))
            {
                // If we have many tab bars here, it will invalidate pointers from pooled tab bars
                for (int i = 0; i < 10; i++)
                    if (ImGui::BeginTabBar(Str30f("Inner TabBar %d", i).c_str()))
                    {
                        if (ImGui::BeginTabItem("Inner TabItem"))
                            ImGui::EndTabItem();
                        ImGui::EndTabBar();
                    }
                ImGui::EndTabItem();
            }
            ImGui::EndTabBar();
        }
        ImGui::End();
    };

    // ## Test BeginTabBar() append
    struct TabBarMultipleSubmissionVars { bool AppendToTabBar = true; ImVec2 CursorAfterActiveTab; ImVec2 CursorAfterFirstBeginTabBar; ImVec2 CursorAfterFirstWidget; ImVec2 CursorAfterSecondBeginTabBar; ImVec2 CursorAfterSecondWidget; ImVec2 CursorAfterSecondEndTabBar; };
    t = IM_REGISTER_TEST(e, "widgets", "widgets_tabbar_append");
    t->SetUserDataType<TabBarMultipleSubmissionVars>();
    t->GuiFunc = [](ImGuiTestContext* ctx)
    {
        ImGuiContext& g = *ctx->UiContext;
        auto& vars = ctx->GetUserData<TabBarMultipleSubmissionVars>();

        ImGui::Begin("Test Window", NULL, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_AlwaysAutoResize);
        ImGui::Checkbox("AppendToTabBar", &vars.AppendToTabBar);
        if (ImGui::BeginTabBar("TabBar"))
        {
            vars.CursorAfterFirstBeginTabBar = g.CurrentWindow->DC.CursorPos;
            if (ImGui::BeginTabItem("Tab 0"))
            {
                ImGui::Text("Tab 0");
                ImGui::EndTabItem();
                vars.CursorAfterActiveTab = g.CurrentWindow->DC.CursorPos;
            }
            if (ImGui::BeginTabItem("Tab 1"))
            {
                for (int i = 0; i < 3; i++)
                    ImGui::Text("Tab 1 Line %d", i);
                ImGui::EndTabItem();
                vars.CursorAfterActiveTab = g.CurrentWindow->DC.CursorPos;
            }
            ImGui::EndTabBar();
        }
        ImGui::Text("After first TabBar submission");
        vars.CursorAfterFirstWidget = g.CurrentWindow->DC.CursorPos;

        if (vars.AppendToTabBar && ImGui::BeginTabBar("TabBar"))
        {
            vars.CursorAfterSecondBeginTabBar = g.CurrentWindow->DC.CursorPos;
            if (ImGui::BeginTabItem("Tab A"))
            {
                ImGui::Text("I'm tab A");
                ImGui::EndTabItem();
                vars.CursorAfterActiveTab = g.CurrentWindow->DC.CursorPos;
            }
            ImGui::EndTabBar();
            vars.CursorAfterSecondEndTabBar = g.CurrentWindow->DC.CursorPos;
        }
        ImGui::Text("After second TabBar submission");
        vars.CursorAfterSecondWidget = g.CurrentWindow->DC.CursorPos;

        ImGui::End();
    };
    t->TestFunc = [](ImGuiTestContext* ctx)
    {
        ImGuiContext& g = *ctx->UiContext;
        auto& vars = ctx->GetUserData<TabBarMultipleSubmissionVars>();

        ctx->SetRef("Test Window/TabBar");

        const float line_height = g.FontSize + g.Style.ItemSpacing.y;
        for (bool append_to_tab_bar : { false, true })
        {
            vars.AppendToTabBar = append_to_tab_bar;
            ctx->Yield();

            for (const char* tab_name : { "Tab 0", "Tab 1", "Tab A" })
            {
                if (!append_to_tab_bar && strcmp(tab_name, "Tab A") == 0)
                    continue;

                ctx->ItemClick(tab_name);
                ctx->Yield();

                float active_tab_height = line_height;
                if (strcmp(tab_name, "Tab 1") == 0)
                    active_tab_height *= 3;

                IM_CHECK(vars.CursorAfterActiveTab.y == vars.CursorAfterFirstBeginTabBar.y + active_tab_height);
                IM_CHECK(vars.CursorAfterFirstWidget.y == vars.CursorAfterActiveTab.y + line_height);
                if (append_to_tab_bar)
                {
                    IM_CHECK(vars.CursorAfterSecondBeginTabBar.y == vars.CursorAfterFirstBeginTabBar.y);
                    IM_CHECK(vars.CursorAfterSecondEndTabBar.y == vars.CursorAfterFirstWidget.y);
                }
                IM_CHECK(vars.CursorAfterSecondWidget.y == vars.CursorAfterFirstWidget.y + line_height);
            }
        }
    };


#ifdef IMGUI_HAS_DOCK
    // ## Test Dockspace within a TabItem
    t = IM_REGISTER_TEST(e, "widgets", "widgets_tabbar_dockspace");
    t->GuiFunc = [](ImGuiTestContext* ctx)
    {
        ImGui::Begin("Test Window", NULL, ImGuiWindowFlags_NoSavedSettings);
        if (ImGui::BeginTabBar("TabBar"))
        {
            if (ImGui::BeginTabItem("TabItem"))
            {
                ImGui::DockSpace(ImGui::GetID("Hello"), ImVec2(0, 0));
                ImGui::EndTabItem();
            }
            ImGui::EndTabBar();
        }
        ImGui::End();
    };
#endif

    // ## Test SetSelected on first frame of a TabItem
    t = IM_REGISTER_TEST(e, "widgets", "widgets_tabbar_tabitem_setselected");
    t->GuiFunc = [](ImGuiTestContext* ctx)
    {
        ImGui::Begin("Test Window", NULL, ImGuiWindowFlags_NoSavedSettings);
        if (ImGui::BeginTabBar("tab_bar"))
        {
            if (ImGui::BeginTabItem("TabItem 0"))
            {
                ImGui::TextUnformatted("First tab content");
                ImGui::EndTabItem();
            }

            if (ctx->FrameCount >= 0)
            {
                bool tab_item_visible = ImGui::BeginTabItem("TabItem 1", NULL, ctx->FrameCount == 0 ? ImGuiTabItemFlags_SetSelected : ImGuiTabItemFlags_None);
                if (tab_item_visible)
                {
                    ImGui::TextUnformatted("Second tab content");
                    ImGui::EndTabItem();
                }
                if (ctx->FrameCount > 0)
                    IM_CHECK(tab_item_visible);
            }
            ImGui::EndTabBar();
        }
        ImGui::End();
    };
    t->TestFunc = [](ImGuiTestContext* ctx) { ctx->Yield(); };

    // ## Tests: Coverage: TabBar: TabBarTabListPopupButton() and TabBarScrollingButtons()
    t = IM_REGISTER_TEST(e, "widgets", "widgets_tabbar_popup_scrolling_button");
    struct TabBarCoveragePopupScrolling { int TabCount = 9; int Selected = -1; };
    t->SetUserDataType<TabBarCoveragePopupScrolling>();
    t->GuiFunc = [](ImGuiTestContext* ctx)
    {
        auto& vars = ctx->GetUserData<TabBarCoveragePopupScrolling>();
        ImGui::SetNextWindowSize(ImVec2(200, 100));
        ImGui::Begin("Test Window", NULL, ImGuiWindowFlags_NoSavedSettings);
        if (ImGui::BeginTabBar("TabBar", ImGuiTabBarFlags_TabListPopupButton | ImGuiTabBarFlags_FittingPolicyScroll))
        {
            for (int i = 0; i < vars.TabCount; i++)
                if (ImGui::BeginTabItem(Str16f{ "Tab %d", i }.c_str(), NULL)) { vars.Selected = i; ImGui::EndTabItem(); }
            ImGui::EndTabBar();
        }
        ImGui::End();
    };
    t->TestFunc = [](ImGuiTestContext* ctx)
    {
        auto& vars = ctx->GetUserData<TabBarCoveragePopupScrolling>();
        ctx->SetRef("Test Window");
        ctx->ItemClick("TabBar/Tab 0"); // Ensure first tab is selected

        for (int i = 0; i < vars.TabCount; i++)
        {
            ctx->ItemClick("TabBar/##<");
            ctx->Yield();
            IM_CHECK_EQ(vars.Selected, i == 0 ? 0 : i - 1);

            ctx->ItemClick("TabBar/##v");
            ctx->ItemClick(Str64f("/##Combo_00/Tab %d", i).c_str());
            ctx->Yield();
            IM_CHECK_EQ(vars.Selected, i);

            ctx->ItemClick("TabBar/##>");
            ctx->Yield();
            IM_CHECK_EQ(vars.Selected, i == vars.TabCount - 1 ? vars.TabCount - 1 : i + 1);
        }

        // Click on all even tab
        for (int i = 0; i < vars.TabCount / 2; i++)
        {
            const int even_i = i * 2;
            ctx->ItemClick(Str64f("TabBar/Tab %d", even_i).c_str());
            IM_CHECK_EQ(vars.Selected, even_i);
        }

        // Click on all odd tab
        for (int i = 0; i < vars.TabCount / 2; i++)
        {
            const int odd_i = i * 2 + 1;
            ctx->ItemClick(Str64f("TabBar/Tab %d", odd_i).c_str());
            IM_CHECK_EQ(vars.Selected, odd_i);
        }
    };

    // ## Test various TreeNode flags
    t = IM_REGISTER_TEST(e, "widgets", "widgets_treenode_behaviors");
    struct TreeNodeTestVars { bool Reset = true, IsOpen = false, IsMultiSelect = false; int ToggleCount = 0; int DragSourceCount = 0;  ImGuiTreeNodeFlags Flags = 0; };
    t->SetUserDataType<TreeNodeTestVars>();
    t->GuiFunc = [](ImGuiTestContext* ctx)
    {
        ImGui::SetNextWindowSize(ImVec2(300, 100), ImGuiCond_Always);
        ImGui::Begin("Test Window", NULL, ImGuiWindowFlags_NoSavedSettings);

        ImGui::ColorButton("Color", ImVec4(1.0f, 1.0f, 0.0f, 1.0f), ImGuiColorEditFlags_NoTooltip); // To test open-on-drag-hold

        TreeNodeTestVars& vars = ctx->GetUserData<TreeNodeTestVars>();
        if (vars.Reset)
        {
            ImGui::GetStateStorage()->SetInt(ImGui::GetID("AAA"), 0);
            vars.ToggleCount = vars.DragSourceCount = 0;
        }
        vars.Reset = false;
        ImGui::Text("Flags: 0x%08X, MultiSelect: %d", vars.Flags, vars.IsMultiSelect);

#ifdef IMGUI_HAS_MULTI_SELECT
        if (vars.IsMultiSelect)
        {
            ImGui::BeginMultiSelect(ImGuiMultiSelectFlags_None, NULL, false); // Placeholder, won't interact properly
            ImGui::SetNextItemSelectionData(NULL);
        }
#endif

        vars.IsOpen = ImGui::TreeNodeEx("AAA", vars.Flags);
        if (ImGui::IsItemToggledOpen())
            vars.ToggleCount++;
        if (ImGui::BeginDragDropSource())
        {
            vars.DragSourceCount++;
            ImGui::SetDragDropPayload("_TREENODE", NULL, 0);
            ImGui::Text("Drag Source Tooltip");
            ImGui::EndDragDropSource();
        }
        if (vars.IsOpen)
        {
            ImGui::Text("Contents");
            ImGui::TreePop();
        }

#ifdef IMGUI_HAS_MULTI_SELECT
        if (vars.IsMultiSelect)
            ImGui::EndMultiSelect();
#endif

        ImGui::End();
    };
    t->TestFunc = [](ImGuiTestContext* ctx)
    {
        TreeNodeTestVars& vars = ctx->GetUserData<TreeNodeTestVars>();
        ctx->SetRef("Test Window");

#ifdef IMGUI_HAS_MULTI_SELECT
        int loop_count = 2;
#else
        int loop_count = 1;
#endif

        for (int loop_n = 0; loop_n < loop_count; loop_n++)
        {
            vars.IsMultiSelect = (loop_n == 1);

            {
                ctx->LogInfo("## ImGuiTreeNodeFlags_None, IsMultiSelect=%d", vars.IsMultiSelect);
                vars.Reset = true;
                vars.Flags = ImGuiTreeNodeFlags_None;
                ctx->Yield();
                IM_CHECK(vars.IsOpen == false && vars.ToggleCount == 0);

                // Click on arrow
                ctx->MouseMove("AAA", ImGuiTestOpFlags_MoveToEdgeL);
                ctx->MouseDown(0); // Toggle on Down when hovering Arrow
                IM_CHECK_EQ(vars.IsOpen, true);
                ctx->MouseUp(0);
                IM_CHECK_EQ(vars.IsOpen, true);
                ctx->MouseClick(0);
                IM_CHECK_EQ(vars.IsOpen, false);
                ctx->MouseDoubleClick(0);
                IM_CHECK_EQ(vars.IsOpen, false);
                IM_CHECK_EQ(vars.ToggleCount, 4);

                // Click on main section (_OpenOnArrow is implicit/automatic with MultiSelect, so Main Section won't react)
                if (!vars.IsMultiSelect)
                {
                    vars.ToggleCount = 0;
                    ctx->MouseMove("AAA");
                    ctx->MouseClick(0);
                    IM_CHECK_EQ_NO_RET(vars.IsOpen, true);
                    ctx->MouseClick(0);
                    IM_CHECK_EQ_NO_RET(vars.IsOpen, false);
                    ctx->MouseDoubleClick(0);
                    IM_CHECK_EQ_NO_RET(vars.IsOpen, false);
                    IM_CHECK_EQ_NO_RET(vars.ToggleCount, 4);
                }

                // Test TreeNode as drag source
                IM_CHECK_EQ(vars.DragSourceCount, 0);
                ctx->ItemDragWithDelta("AAA", ImVec2(50, 50));
                IM_CHECK_GT(vars.DragSourceCount, 0);
                IM_CHECK_EQ(vars.IsOpen, false);

                // Test TreeNode opening on drag-hold
                ctx->ItemDragOverAndHold("Color", "AAA");
                IM_CHECK_EQ(vars.IsOpen, true);
            }

            {
                ctx->LogInfo("## ImGuiTreeNodeFlags_OpenOnDoubleClick, IsMultiSelect=%d", vars.IsMultiSelect);
                vars.Reset = true;
                vars.Flags = ImGuiTreeNodeFlags_OpenOnDoubleClick;
                ctx->Yield();
                IM_CHECK(vars.IsOpen == false && vars.ToggleCount == 0);

                // Click on arrow (_OpenOnArrow is implicit/automatic with MultiSelect)
                if (!vars.IsMultiSelect)
                {
                    ctx->MouseMove("AAA", ImGuiTestOpFlags_MoveToEdgeL);
                    ctx->MouseDown(0);
                    IM_CHECK_EQ(vars.IsOpen, false);
                    ctx->MouseUp(0);
                    IM_CHECK_EQ(vars.IsOpen, false);
                    ctx->MouseClick(0);
                    IM_CHECK_EQ(vars.IsOpen, false);
                    IM_CHECK_EQ(vars.ToggleCount, 0);
                    ctx->MouseDoubleClick(0);
                    IM_CHECK_EQ(vars.IsOpen, true);
                    ctx->MouseDoubleClick(0);
                    IM_CHECK_EQ(vars.IsOpen, false);
                    IM_CHECK_EQ(vars.ToggleCount, 2);
                }

                // Double-click on main section
                vars.ToggleCount = 0;
                ctx->MouseMove("AAA");
                ctx->MouseClick(0);
                IM_CHECK_EQ(vars.IsOpen, false);
                ctx->MouseClick(0);
                IM_CHECK_EQ(vars.IsOpen, false);
                IM_CHECK_EQ(vars.ToggleCount, 0);
                ctx->MouseDoubleClick(0);
                IM_CHECK_EQ(vars.IsOpen, true);
                ctx->MouseDoubleClick(0);
                IM_CHECK_EQ(vars.IsOpen, false);
                IM_CHECK_EQ(vars.ToggleCount, 2);

                // Test TreeNode as drag source
                IM_CHECK_EQ(vars.DragSourceCount, 0);
                ctx->ItemDragWithDelta("AAA", ImVec2(50, 50));
                IM_CHECK_GT(vars.DragSourceCount, 0);
                IM_CHECK_EQ(vars.IsOpen, false);

                // Test TreeNode opening on drag-hold
                ctx->ItemDragOverAndHold("Color", "AAA");
                IM_CHECK_EQ(vars.IsOpen, true);
            }

            {
                ctx->LogInfo("## ImGuiTreeNodeFlags_OpenOnArrow, IsMultiSelect=%d", vars.IsMultiSelect);
                vars.Reset = true;
                vars.Flags = ImGuiTreeNodeFlags_OpenOnArrow;
                ctx->Yield();
                IM_CHECK(vars.IsOpen == false && vars.ToggleCount == 0);

                // Click on arrow
                ctx->MouseMove("AAA", ImGuiTestOpFlags_MoveToEdgeL);
                ctx->MouseDown(0);
                IM_CHECK_EQ(vars.IsOpen, true);
                ctx->MouseUp(0);
                IM_CHECK_EQ(vars.IsOpen, true);
                ctx->MouseClick(0);
                IM_CHECK_EQ(vars.IsOpen, false);
                IM_CHECK_EQ(vars.ToggleCount, 2);
                ctx->MouseDoubleClick(0);
                IM_CHECK_EQ(vars.IsOpen, false);
                IM_CHECK_EQ(vars.ToggleCount, 4);

                // Click on main section
                vars.ToggleCount = 0;
                ctx->MouseMove("AAA");
                ctx->MouseClick(0);
                IM_CHECK_EQ(vars.IsOpen, false);
                ctx->MouseClick(0);
                IM_CHECK_EQ(vars.IsOpen, false);
                IM_CHECK_EQ(vars.ToggleCount, 0);
                ctx->MouseDoubleClick(0);
                IM_CHECK_EQ(vars.IsOpen, false);
                IM_CHECK_EQ(vars.ToggleCount, 0);

                // Test TreeNode as drag source
                IM_CHECK_EQ(vars.DragSourceCount, 0);
                ctx->ItemDragWithDelta("AAA", ImVec2(50, 50));
                IM_CHECK_GT(vars.DragSourceCount, 0);
                IM_CHECK_EQ(vars.IsOpen, false);

                // Test TreeNode opening on drag-hold
                ctx->ItemDragOverAndHold("Color", "AAA");
                IM_CHECK_EQ(vars.IsOpen, true);
            }

            {
                ctx->LogInfo("## ImGuiTreeNodeFlags_OpenOnArrow|ImGuiTreeNodeFlags_OpenOnDoubleClick, IsMultiSelect=%d", vars.IsMultiSelect);
                vars.Reset = true;
                vars.Flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;
                ctx->Yield();
                IM_CHECK(vars.IsOpen == false && vars.ToggleCount == 0);

                // Click on arrow
                ctx->MouseMove("AAA", ImGuiTestOpFlags_MoveToEdgeL);
                ctx->MouseDown(0);
                IM_CHECK_EQ(vars.IsOpen, true);
                ctx->MouseUp(0);
                ctx->MouseClick(0);
                IM_CHECK_EQ(vars.IsOpen, false);
                IM_CHECK_EQ(vars.ToggleCount, 2);
                ctx->MouseDoubleClick(0);
                IM_CHECK_EQ(vars.IsOpen, false);
                IM_CHECK_EQ(vars.ToggleCount, 4);

                // Click on main section
                vars.ToggleCount = 0;
                ctx->MouseMove("AAA");
                ctx->MouseClick(0);
                IM_CHECK_EQ(vars.IsOpen, false);
                ctx->MouseClick(0);
                IM_CHECK_EQ(vars.IsOpen, false);
                IM_CHECK_EQ(vars.ToggleCount, 0);
                ctx->MouseDoubleClick(0);
                IM_CHECK_EQ(vars.IsOpen, true);
                ctx->MouseDoubleClick(0);
                IM_CHECK_EQ(vars.IsOpen, false);
                IM_CHECK_EQ(vars.ToggleCount, 2);

                // Test TreeNode as drag source
                IM_CHECK_EQ(vars.DragSourceCount, 0);
                ctx->ItemDragWithDelta("AAA", ImVec2(50, 50));
                IM_CHECK_GT(vars.DragSourceCount, 0);
                IM_CHECK_EQ(vars.IsOpen, false);

                // Test TreeNode opening on drag-hold
                ctx->ItemDragOverAndHold("Color", "AAA");
                IM_CHECK_EQ(vars.IsOpen, true);
            }
        }
    };

    // ## Test ImGuiTreeNodeFlags_SpanAvailWidth and ImGuiTreeNodeFlags_SpanFullWidth flags
    t = IM_REGISTER_TEST(e, "widgets", "widgets_treenode_span_width");
    t->GuiFunc = [](ImGuiTestContext* ctx)
    {
        ImGui::SetNextWindowSize(ImVec2(300, 100), ImGuiCond_Always);
        ImGui::Begin("Test Window", NULL, ImGuiWindowFlags_NoSavedSettings);

        ImGuiContext& g = *ImGui::GetCurrentContext();
        ImGuiWindow* window = ImGui::GetCurrentWindow();

        ImGui::SetNextItemOpen(true);
        if (ImGui::TreeNodeEx("Parent"))
        {
            // Interaction rect zoes not span entire width of work area.
            IM_CHECK(g.LastItemData.Rect.Max.x < window->WorkRect.Max.x);
            // But it starts at very beginning of WorkRect for first tree level.
            IM_CHECK(g.LastItemData.Rect.Min.x == window->WorkRect.Min.x);
            ImGui::SetNextItemOpen(true);
            if (ImGui::TreeNodeEx("Regular"))
            {
                // Interaction rect does not span entire width of work area.
                IM_CHECK(g.LastItemData.Rect.Max.x < window->WorkRect.Max.x);
                IM_CHECK(g.LastItemData.Rect.Min.x > window->WorkRect.Min.x);
                ImGui::TreePop();
            }
            ImGui::SetNextItemOpen(true);
            if (ImGui::TreeNodeEx("SpanAvailWidth", ImGuiTreeNodeFlags_SpanAvailWidth))
            {
                // Interaction rect matches visible frame rect
                IM_CHECK((g.LastItemData.StatusFlags & ImGuiItemStatusFlags_HasDisplayRect) != 0);
                IM_CHECK(g.LastItemData.DisplayRect.Min == g.LastItemData.Rect.Min);
                IM_CHECK(g.LastItemData.DisplayRect.Max == g.LastItemData.Rect.Max);
                // Interaction rect extends to the end of the available area.
                IM_CHECK(g.LastItemData.Rect.Max.x == window->WorkRect.Max.x);
                ImGui::TreePop();
            }
            ImGui::SetNextItemOpen(true);
            if (ImGui::TreeNodeEx("SpanFullWidth", ImGuiTreeNodeFlags_SpanFullWidth))
            {
                // Interaction rect matches visible frame rect
                IM_CHECK((g.LastItemData.StatusFlags & ImGuiItemStatusFlags_HasDisplayRect) != 0);
                IM_CHECK(g.LastItemData.DisplayRect.Min == g.LastItemData.Rect.Min);
                IM_CHECK(g.LastItemData.DisplayRect.Max == g.LastItemData.Rect.Max);
                // Interaction rect extends to the end of the available area.
                IM_CHECK(g.LastItemData.Rect.Max.x == window->WorkRect.Max.x);
                // ImGuiTreeNodeFlags_SpanFullWidth also extends interaction rect to the left.
                IM_CHECK(g.LastItemData.Rect.Min.x == window->WorkRect.Min.x);
                ImGui::TreePop();
            }
            ImGui::TreePop();
        }

        ImGui::End();
    };

    // ## Test PlotLines() with a single value (#2387).
    t = IM_REGISTER_TEST(e, "widgets", "widgets_plot_lines_unexpected_input");
    t->GuiFunc = [](ImGuiTestContext* ctx)
    {
        float values[1] = {0.f};
        ImGui::PlotLines("PlotLines 1", NULL, 0);
        ImGui::PlotLines("PlotLines 2", values, 0);
        ImGui::PlotLines("PlotLines 3", values, 1);
        // FIXME-TESTS: If test did not crash - it passed. A better way to check this would be useful.
    };

    // ## Test ColorEdit basic Drag and Drop
    t = IM_REGISTER_TEST(e, "widgets", "widgets_drag_coloredit");
    t->GuiFunc = [](ImGuiTestContext* ctx)
    {
        ImGuiTestGenericVars& vars = ctx->GenericVars;
        ImGui::SetNextWindowSize(ImVec2(300, 200));
        ImGui::Begin("Test Window", NULL, ImGuiWindowFlags_NoSavedSettings);
        ImGui::ColorEdit4("ColorEdit1", &vars.Vec4Array[0].x, ImGuiColorEditFlags_None);
        ImGui::ColorEdit4("ColorEdit2", &vars.Vec4Array[1].x, ImGuiColorEditFlags_None);
        ImGui::End();
    };
    t->TestFunc = [](ImGuiTestContext* ctx)
    {
        ImGuiTestGenericVars& vars = ctx->GenericVars;
        vars.Vec4Array[0] = ImVec4(1, 0, 0, 1);
        vars.Vec4Array[1] = ImVec4(0, 1, 0, 1);

        ctx->SetRef("Test Window");

        IM_CHECK_NE(memcmp(&vars.Vec4Array[0], &vars.Vec4Array[1], sizeof(ImVec4)), 0);
        ctx->ItemDragAndDrop("ColorEdit1/##ColorButton", "ColorEdit2/##X"); // FIXME-TESTS: Inner items
        IM_CHECK_EQ(memcmp(&vars.Vec4Array[0], &vars.Vec4Array[1], sizeof(ImVec4)), 0);
    };

    // ## Test BeginDragDropSource() with NULL id.
    t = IM_REGISTER_TEST(e, "widgets", "widgets_drag_source_null_id");
    struct WidgetDragSourceNullIDData
    {
        ImVec2 Source;
        ImVec2 Destination;
        bool Dropped = false;
    };
    t->SetUserDataType<WidgetDragSourceNullIDData>();
    t->GuiFunc = [](ImGuiTestContext* ctx)
    {
        WidgetDragSourceNullIDData& user_data = *(WidgetDragSourceNullIDData*)ctx->UserData;

        ImGui::Begin("Null ID Test", NULL, ImGuiWindowFlags_NoSavedSettings);
        ImGui::TextUnformatted("Null ID");
        user_data.Source = ImRect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax()).GetCenter();

        if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
        {
            int magic = 0xF00;
            ImGui::SetDragDropPayload("MAGIC", &magic, sizeof(int));
            ImGui::EndDragDropSource();
        }
        ImGui::TextUnformatted("Drop Here");
        user_data.Destination = ImRect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax()).GetCenter();

        if (ImGui::BeginDragDropTarget())
        {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("MAGIC"))
            {
                user_data.Dropped = true;
                IM_CHECK_EQ(payload->DataSize, (int)sizeof(int));
                IM_CHECK_EQ(*(int*)payload->Data, 0xF00);
            }
            ImGui::EndDragDropTarget();
        }
        ImGui::End();
    };
    t->TestFunc = [](ImGuiTestContext* ctx)
    {
        WidgetDragSourceNullIDData& user_data = *(WidgetDragSourceNullIDData*)ctx->UserData;

        // ImGui::TextUnformatted() does not have an ID therefore we can not use ctx->ItemDragAndDrop() as that refers
        // to items by their ID.
        ctx->MouseMoveToPos(user_data.Source);
        ctx->SleepShort();
        ctx->MouseDown(0);

        ctx->MouseMoveToPos(user_data.Destination);
        ctx->SleepShort();
        ctx->MouseUp(0);

        IM_CHECK(user_data.Dropped);
    };

    // ## Test preserving g.ActiveId during drag operation opening tree items.
    t = IM_REGISTER_TEST(e, "widgets", "widgets_drag_hold_to_open");
    t->GuiFunc = [](ImGuiTestContext* ctx)
    {
        if (ImGui::Begin("Test Window", NULL, ImGuiWindowFlags_NoSavedSettings))
        {
            ImGui::Button("Drag");
            if (ImGui::BeginDragDropSource())
            {
                int magic = 0xF00;
                ImGui::SetDragDropPayload("MAGIC", &magic, sizeof(int));
                ImGui::EndDragDropSource();
            }
        }
        ImGui::End();
    };
    t->TestFunc = [](ImGuiTestContext* ctx)
    {
        ImGuiContext& g = *ctx->UiContext;

        ctx->SetRef("Dear ImGui Demo");
        ctx->ItemCloseAll("");
        ctx->WindowFocus(""); // So it's under TestWindow
        // FIXME-TESTS: This can still fail if "Test Window" covers "Dear ImGui Demo"

        ctx->SetRef("Test Window");
        ImGuiID active_id = ctx->GetID("Drag");
        ctx->MouseMove("Drag");
        ctx->SleepShort();
        ctx->MouseDown();
        ctx->MouseLiftDragThreshold();
        IM_CHECK_EQ(g.ActiveId, active_id);

        ctx->SetRef("Dear ImGui Demo");
        ctx->MouseMove("Widgets", ImGuiTestOpFlags_NoFocusWindow);
        ctx->SleepNoSkip(1.0f, 1.0f / 60.0f);

        IM_CHECK(ctx->ItemInfo("Widgets") != NULL);
        IM_CHECK((ctx->ItemInfo("Widgets")->StatusFlags & ImGuiItemStatusFlags_Opened) != 0);
        IM_CHECK_EQ(g.ActiveId, active_id);
        ctx->MouseMove("Trees", ImGuiTestOpFlags_NoFocusWindow);
        ctx->SleepNoSkip(1.0f, 1.0f / 60.0f);
        IM_CHECK((ctx->ItemInfo("Trees")->StatusFlags & ImGuiItemStatusFlags_Opened) != 0);
        IM_CHECK_EQ(g.ActiveId, active_id);
        ctx->MouseUp(0);
    };

    // ## Test overlapping drag and drop targets. The drag and drop system always prioritize the smaller target.
    t = IM_REGISTER_TEST(e, "widgets", "widgets_drag_overlapping_targets");
    t->GuiFunc = [](ImGuiTestContext* ctx)
    {
        ImGui::Begin("Test Window", NULL, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_AlwaysAutoResize);

        ImGui::Button("Drag");
        if (ImGui::BeginDragDropSource())
        {
            int value = 0xF00D;
            ImGui::SetDragDropPayload("_TEST_VALUE", &value, sizeof(int));
            ImGui::EndDragDropSource();
        }

        auto render_button = [](ImGuiTestContext* ctx, const char* name, const ImVec2& pos, const ImVec2& size)
        {
            ImGui::SetCursorScreenPos(pos);
            ImGui::Button(name, size);
            if (ImGui::BeginDragDropTarget())
            {
                if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("_TEST_VALUE"))
                {
                    IM_UNUSED(payload);
                    ctx->GenericVars.Id = ImGui::GetItemID();
                }
                ImGui::EndDragDropTarget();
            }
        };

        // Render small button over big one
        ImVec2 pos = ImGui::GetCursorScreenPos();
        render_button(ctx, "Big1", pos, ImVec2(100, 100));
        render_button(ctx, "Small1", pos + ImVec2(25, 25), ImVec2(50, 50));

        // Render small button over small one
        render_button(ctx, "Small2", pos + ImVec2(0, 110) + ImVec2(25, 25), ImVec2(50, 50));
        render_button(ctx, "Big2", pos + ImVec2(0, 110), ImVec2(100, 100));

        ImGui::End();
    };
    t->TestFunc = [](ImGuiTestContext* ctx)
    {
        ctx->SetRef("Test Window");

        ctx->GenericVars.Id = 0;
        ctx->ItemDragAndDrop("Drag", "Small1");
        IM_CHECK(ctx->GenericVars.Id == ctx->GetID("Small1"));

        ctx->GenericVars.Id = 0;
        ctx->ItemDragAndDrop("Drag", "Small2");
        IM_CHECK(ctx->GenericVars.Id == ctx->GetID("Small2"));
    };

    // ## Test drag sources with _SourceNoPreviewTooltip flag not producing a tooltip.
    // ## Test drag target/accept with ImGuiDragDropFlags_AcceptNoPreviewTooltip
    t = IM_REGISTER_TEST(e, "widgets", "widgets_drag_no_preview_tooltip");
    struct DragNoPreviewTooltipVars { bool TooltipWasVisible = false; bool TooltipIsVisible = false; ImGuiDragDropFlags AcceptFlags = 0; };
    t->SetUserDataType<DragNoPreviewTooltipVars>();
    t->GuiFunc = [](ImGuiTestContext* ctx)
    {
        DragNoPreviewTooltipVars& vars = ctx->GetUserData<DragNoPreviewTooltipVars>();

        ImGui::Begin("Test Window", NULL, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_AlwaysAutoResize);

        auto create_drag_drop_source = [](ImGuiDragDropFlags flags)
        {
            if (ImGui::BeginDragDropSource(flags))
            {
                int value = 0xF00D;
                ImGui::SetDragDropPayload("_TEST_VALUE", &value, sizeof(int));
                ImGui::EndDragDropSource();
            }
        };

        ImGui::Button("Drag");
        create_drag_drop_source(ImGuiDragDropFlags_SourceNoPreviewTooltip);

        ImGui::Button("Drag Extern");
        if (ImGui::IsItemClicked())
            create_drag_drop_source(ImGuiDragDropFlags_SourceNoPreviewTooltip | ImGuiDragDropFlags_SourceExtern);

        ImGui::Button("Drag Accept");
        create_drag_drop_source(0);

        ImGui::Button("Drop");
        if (ImGui::BeginDragDropTarget())
        {
            ImGui::AcceptDragDropPayload("_TEST_VALUE", vars.AcceptFlags);
            ImGui::EndDragDropTarget();
        }

        ImGuiContext& g = *ctx->UiContext;
        ImGuiWindow* tooltip = ctx->GetWindowByRef(Str16f("##Tooltip_%02d", g.TooltipOverrideCount).c_str());
        vars.TooltipIsVisible = g.TooltipOverrideCount != 0 || (tooltip != NULL && (tooltip->Active || tooltip->WasActive));
        vars.TooltipWasVisible |= vars.TooltipIsVisible;

        ImGui::End();
    };
    t->TestFunc = [](ImGuiTestContext* ctx)
    {
        DragNoPreviewTooltipVars& vars = ctx->GetUserData<DragNoPreviewTooltipVars>();
        ctx->SetRef("Test Window");
        ctx->ItemDragAndDrop("Drag", "Drop");
        IM_CHECK(vars.TooltipWasVisible == false);
        vars.TooltipWasVisible = false;
        ctx->ItemDragAndDrop("Drag Extern", "Drop");
        IM_CHECK(vars.TooltipWasVisible == false);
        vars.TooltipWasVisible = false;

        vars.AcceptFlags = 0;
        ctx->ItemDragOverAndHold("Drag Accept", "Drop");
        //ctx->Yield();   // A visible tooltip window gets hidden with one frame delay. (due to how we test for Active || WasActive)
        IM_CHECK(vars.TooltipWasVisible == true);
        IM_CHECK(vars.TooltipIsVisible == true);
        ctx->MouseUp();
        vars.TooltipWasVisible = false;

        vars.AcceptFlags = ImGuiDragDropFlags_AcceptNoPreviewTooltip;
        ctx->ItemDragOverAndHold("Drag Accept", "Drop");
        ctx->Yield();   // A visible tooltip window gets hidden with one frame delay (due to how we test for Active || WasActive)
        IM_CHECK(vars.TooltipWasVisible == true);
        IM_CHECK(vars.TooltipIsVisible == false);
        ctx->MouseUp();
    };

    // ## Test using default context menu along with a combo (#4167)
#if IMGUI_VERSION_NUM >= 18211
    t = IM_REGISTER_TEST(e, "widgets", "widgets_combo_context_menu");
    t->GuiFunc = [](ImGuiTestContext* ctx)
    {
        ImGui::Begin("Test Window", NULL, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_AlwaysAutoResize);

        if (ImGui::BeginCombo("Combo", "Preview"))
        {
            ctx->GenericVars.Bool1 = true;
            ImGui::Selectable("Close");
            ImGui::EndCombo();
        }
        if (ImGui::BeginPopupContextItem())
        {
            if (ImGui::Button("Close"))
                ImGui::CloseCurrentPopup();
            ctx->GenericVars.Bool2 = true;
            ImGui::EndPopup();
        }
        ImGui::End();
    };
    t->TestFunc = [](ImGuiTestContext* ctx)
    {
        ctx->SetRef("Test Window");
        ctx->MouseMove("Combo");
        ctx->MouseClick(ImGuiMouseButton_Right);
        IM_CHECK(ctx->GenericVars.Bool1 == false && ctx->GenericVars.Bool2 == true);
        ctx->SetRef(ctx->GetFocusWindowRef());
        ctx->ItemClick("Close");
        ctx->GenericVars.Bool1 = ctx->GenericVars.Bool2 = false;

        ctx->SetRef("Test Window");
        ctx->ItemClick("Combo");
        IM_CHECK(ctx->GenericVars.Bool1 == true && ctx->GenericVars.Bool2 == false);
        ctx->SetRef(ctx->GetFocusWindowRef());
        ctx->ItemClick("Close");
        ctx->GenericVars.Bool1 = ctx->GenericVars.Bool2 = false;
    };
#endif

#if IMGUI_VERSION_NUM >= 18308
    // ## Test BeginComboPreview() and ImGuiComboFlags_CustomPreview
    t = IM_REGISTER_TEST(e, "widgets", "widgets_combo_custom_preview");
    t->GuiFunc = [](ImGuiTestContext* ctx)
    {
        const char* items[] = { "AAAA", "BBBB", "CCCC", "DDDD", "EEEE" };

        ImGuiContext& g = *ImGui::GetCurrentContext();
        ImGui::Begin("Test Window", NULL, ImGuiWindowFlags_NoSavedSettings);
        ImGui::Text("Custom preview:");

        if (ctx->GenericVars.Step == 2)
            ImGui::SetNextItemWidth(ImGui::GetFrameHeight() * 2.0f);

        const ImVec2 color_square_size = ImVec2(ImGui::GetFontSize(), ImGui::GetFontSize());
        bool open = ImGui::BeginCombo("combo custom", "", ImGuiComboFlags_CustomPreview);
        ImRect combo_r = g.LastItemData.Rect;
        if (open)
        {
            for (int n = 0; n < 5; n++)
            {
                ImGui::PushID(n);
                ImGui::ColorButton("##color", ImVec4(1.0f, 0.5f, 0.5f, 1.0f), ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoDragDrop, color_square_size);
                ImGui::SameLine();
                ImGui::Selectable(items[n]);
                ImGui::PopID();
            }
            ImGui::EndCombo();
        }
        ImGuiWindow* window = ImGui::GetCurrentWindow();
        int old_cmd_buffer_size = window->DrawList->CmdBuffer.Size;

        if (ImGui::BeginComboPreview())
        {
            ImGui::ColorButton("##color", ImVec4(1.0f, 0.5f, 0.5f, 1.0f), ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoDragDrop, color_square_size);
            ImGui::TextUnformatted(items[0]);
            ImGui::EndComboPreview();
        }

        // Test sameline restore behavior
        ImGui::SameLine();
        IM_CHECK_EQ(combo_r.Min.y, window->DC.CursorPos.y);
        IM_CHECK_EQ(ImGui::GetStyle().FramePadding.y, window->DC.CurrLineTextBaseOffset);
        ImGui::Text("HELLO");

        // Test draw call merging behavior
        if (ctx->GenericVars.Step == 1)
            IM_CHECK_EQ(old_cmd_buffer_size, window->DrawList->CmdBuffer.Size);
        if (ctx->GenericVars.Step == 2)
            IM_CHECK_EQ(old_cmd_buffer_size + 2, window->DrawList->CmdBuffer.Size);

        ImGui::End();
    };
    t->TestFunc = [](ImGuiTestContext* ctx)
    {
        ctx->SetRef("Test Window");

        ctx->GenericVars.Step = 0;
        ctx->WindowResize("", ImVec2(400, 400));
        ctx->GenericVars.Step = 1;
        ctx->Yield(2);
        ctx->GenericVars.Step = 2;
        ctx->Yield(2);
    };
#endif

#if IMGUI_VERSION_NUM >= 18408
    // ## Test null range in TextUnformatted() (#3615)
    t = IM_REGISTER_TEST(e, "widgets", "widgets_text_null");
    t->GuiFunc = [](ImGuiTestContext* ctx)
    {
        const char* str = "hello world";
        ImGui::Begin("Test Window", NULL, ImGuiWindowFlags_NoSavedSettings);
        ImVec2 p0 = ImGui::GetCursorPos();
        ImGui::TextUnformatted(str, str + 5);
        ImGui::TextUnformatted(str + 1, str + 1);
        ImGui::TextUnformatted(NULL);
        ImVec2 p1 = ImGui::GetCursorPos();
        IM_CHECK_EQ(p0.y + ImGui::GetTextLineHeightWithSpacing() * 3, p1.y);
        ImGui::End();
    };
#endif

    // ## Test long text rendering in TextUnformatted()
    t = IM_REGISTER_TEST(e, "widgets", "widgets_text_long");
    t->TestFunc = [](ImGuiTestContext* ctx)
    {
        ctx->SetRef("Dear ImGui Demo");
        ctx->MenuCheck("Examples/Long text display");
        ctx->SetRef("Example: Long text display");
        ctx->ItemClick("Add 1000 lines");
        ctx->SleepShort();

        Str64f title("/Example: Long text display\\/Log_%08X", ctx->GetID("Log"));
        ImGuiWindow* log_panel = ctx->GetWindowByRef(title.c_str());
        IM_CHECK(log_panel != NULL);
        ImGui::SetScrollY(log_panel, log_panel->ScrollMax.y);
        ctx->SleepShort();
        ctx->ItemClick("Clear");
        // FIXME-TESTS: A bit of extra testing that will be possible once tomato problem is solved.
        // ctx->ComboClick("Test type/Single call to TextUnformatted()");
        // ctx->ComboClick("Test type/Multiple calls to Text(), clipped");
        // ctx->ComboClick("Test type/Multiple calls to Text(), not clipped (slow)");
        ctx->WindowClose("");
    };

    // ## Test LabelText() variants layout (#4004)
    t = IM_REGISTER_TEST(e, "widgets", "widgets_label_text");
    t->GuiFunc = [](ImGuiTestContext* ctx)
    {
        ImGuiContext& g = *ImGui::GetCurrentContext();

        ImGui::SetNextWindowSize(ImVec2(500, 500), ImGuiCond_Appearing);
        ImGui::Begin("Test Window", NULL, ImGuiWindowFlags_NoSavedSettings);

        ImGui::Separator();
        ImGui::LabelText("Single line label", "Single line text");
        IM_CHECK_EQ(g.LastItemData.Rect.GetHeight(), ImGui::GetFrameHeight());

        ImGui::Separator();
        ImGui::LabelText("Multi\n line\n label", "Single line text");
        IM_CHECK_EQ(g.LastItemData.Rect.GetHeight(), ImGui::GetTextLineHeight() * 3.0f + ImGui::GetStyle().FramePadding.y * 2.0f);

        ImGui::Separator();
        ImGui::LabelText("Single line label", "Multi\n line\n text");
        IM_CHECK_EQ(g.LastItemData.Rect.GetHeight(), ImGui::GetTextLineHeight() * 3.0f + ImGui::GetStyle().FramePadding.y * 2.0f);

        ImGui::End();
    };

    // ## Test menu appending.
    t = IM_REGISTER_TEST(e, "widgets", "widgets_menu_append");
    t->GuiFunc = [](ImGuiTestContext* ctx)
    {
        ImGui::Begin("Append Menus", NULL, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_MenuBar);
        ImGui::BeginMenuBar();

        // Menu that we will append to.
        if (ImGui::BeginMenu("First Menu"))
        {
            ImGui::MenuItem("1 First");
            if (ImGui::BeginMenu("Second Menu"))
            {
                ImGui::MenuItem("2 First");
                ImGui::EndMenu();
            }
            ImGui::EndMenu();
        }

        // Append to first menu.
        if (ImGui::BeginMenu("First Menu"))
        {
            if (ImGui::MenuItem("1 Second"))
                ctx->GenericVars.Bool1 = true;
            if (ImGui::BeginMenu("Second Menu"))
            {
                ImGui::MenuItem("2 Second");
                ImGui::EndMenu();
            }
            ImGui::EndMenu();
        }

        ImGui::EndMenuBar();
        ImGui::End();
    };
    t->TestFunc = [](ImGuiTestContext* ctx)
    {
        ctx->SetRef("Append Menus");
        ctx->MenuClick("First Menu");
        ctx->MenuClick("First Menu/1 First");
        IM_CHECK_EQ(ctx->GenericVars.Bool1, false);
        ctx->MenuClick("First Menu/1 Second");
        IM_CHECK_EQ(ctx->GenericVars.Bool1, true);
        ctx->MenuClick("First Menu/Second Menu/2 First");
        ctx->MenuClick("First Menu/Second Menu/2 Second");
    };

    // ## Test main menubar appending.
    t = IM_REGISTER_TEST(e, "widgets", "widgets_main_menubar_append");
    t->GuiFunc = [](ImGuiTestContext* ctx)
    {
        // Menu that we will append to.
        if (ImGui::BeginMainMenuBar())
        {
            if (ImGui::BeginMenu("First Menu"))
                ImGui::EndMenu();
            ImGui::EndMainMenuBar();
        }

        // Append to first menu.
        if (ImGui::BeginMainMenuBar())
        {
            if (ImGui::BeginMenu("Second Menu"))
            {
                if (ImGui::MenuItem("Second"))
                    ctx->GenericVars.Bool1 = true;
                ImGui::EndMenu();
            }
            ImGui::EndMainMenuBar();
        }
    };
    t->TestFunc = [](ImGuiTestContext* ctx)
    {
        ctx->SetRef("##MainMenuBar");
        ctx->MenuClick("Second Menu/Second");
        IM_CHECK_EQ(ctx->GenericVars.Bool1, true);
    };

    // ## Test main menubar navigation
    t = IM_REGISTER_TEST(e, "widgets", "widgets_main_menubar_navigation");
    t->GuiFunc = [](ImGuiTestContext* ctx)
    {
        if (ImGui::BeginMainMenuBar())
        {
            if (ImGui::BeginMenu("Menu 1"))
            {
                if (ImGui::BeginMenu("Sub Menu 1"))
                {
                    ImGui::MenuItem("Item 1");
                    ImGui::EndMenu();
                }
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Menu 2"))
            {
                if (ImGui::BeginMenu("Sub Menu 2-1"))
                {
                    ImGui::MenuItem("Item 2");
                    ImGui::EndMenu();
                }
                if (ImGui::BeginMenu("Sub Menu 2-2"))
                {
                    ImGui::MenuItem("Item 3");
                    ImGui::EndMenu();
                }
                ImGui::EndMenu();
            }
            ImGui::EndMenuBar();
        }
        ImGui::End();
    };
    t->TestFunc = [](ImGuiTestContext* ctx)
    {
        ImGuiContext& g = *ImGui::GetCurrentContext();

        ctx->ItemClick("##MainMenuBar##menubar/Menu 1");

        // Click doesn't affect g.NavId which is null at this point

        // Test basic key Navigation
        ctx->NavKeyPress(ImGuiNavInput_KeyDown_);
        IM_CHECK_EQ(g.NavId, ctx->GetID("##Menu_00/Sub Menu 1"));
        ctx->NavKeyPress(ImGuiNavInput_KeyRight_);
        IM_CHECK_EQ(g.NavId, ctx->GetID("##Menu_01/Item 1"));
        ctx->NavKeyPress(ImGuiNavInput_KeyLeft_);
        IM_CHECK_EQ(g.NavId, ctx->GetID("##Menu_00/Sub Menu 1"));
        ctx->NavKeyPress(ImGuiNavInput_KeyLeft_);
        IM_CHECK_EQ(g.NavId, ctx->GetID("##MainMenuBar##menubar/Menu 1"));

        ctx->MouseMove("##MainMenuBar##menubar/Menu 2"); // FIXME-TESTS: Workaround so TestEngine can find again Menu 1

        // Test forwarding of nav event to parent when there's no match in the current menu
        // Going from "Menu 1/Sub Menu 1/Item 1" to "Menu 2"
        ctx->ItemClick("##MainMenuBar##menubar/Menu 1");
        ctx->NavKeyPress(ImGuiNavInput_KeyDown_);
        ctx->NavKeyPress(ImGuiNavInput_KeyRight_);
        IM_CHECK_EQ(g.NavId, ctx->GetID("##Menu_01/Item 1"));
        ctx->NavKeyPress(ImGuiNavInput_KeyRight_);
        IM_CHECK_EQ(g.NavId, ctx->GetID("##MainMenuBar##menubar/Menu 2"));

        ctx->NavKeyPress(ImGuiNavInput_KeyDown_);
        IM_CHECK_EQ(g.NavId, ctx->GetID("##Menu_00/Sub Menu 2-1"));
        ctx->NavKeyPress(ImGuiNavInput_KeyRight_);
        IM_CHECK_EQ(g.NavId, ctx->GetID("##Menu_01/Item 2"));

        ctx->NavKeyPress(ImGuiNavInput_KeyLeft_);
        ctx->NavKeyPress(ImGuiNavInput_KeyDown_);
        IM_CHECK_EQ(g.NavId, ctx->GetID("##Menu_00/Sub Menu 2-2"));
        ctx->NavKeyPress(ImGuiNavInput_KeyRight_);
        IM_CHECK_EQ(g.NavId, ctx->GetID("##Menu_01/Item 3"));
    };

#ifdef IMGUI_HAS_MULTI_SELECT
    // ## Test MultiSelect API
    struct ExampleSelection
    {
        ImGuiStorage                        Storage;
        int                                 SelectionSize;  // Number of selected items (== number of 1 in the Storage, maintained by this class)
        int                                 RangeRef;       // Reference/pivot item (generally last clicked item)

        ExampleSelection()                  { RangeRef = 0; Clear(); }
        void Clear()                        { Storage.Clear(); SelectionSize = 0; }
        bool GetSelected(int n) const       { return Storage.GetInt((ImGuiID)n, 0) != 0; }
        void SetSelected(int n, bool v)     { int* p_int = Storage.GetIntRef((ImGuiID)n, 0); if (*p_int == (int)v) return; if (v) SelectionSize++; else SelectionSize--; *p_int = (bool)v; }
        int  GetSelectionSize() const       { return SelectionSize; }

        // When using SelectAll() / SetRange() we assume that our objects ID are indices.
        // In this demo we always store selection using indices and never in another manner (e.g. object ID or pointers).
        // If your selection system is storing selection using object ID and you want to support Shift+Click range-selection,
        // you will need a way to iterate from one object to another given the ID you use.
        // You are likely to need some kind of data structure to convert 'view index' <> 'object ID'.
        // FIXME-MULTISELECT: Would be worth providing a demo of doing this.
        // FIXME-MULTISELECT: SetRange() is currently very inefficient since it doesn't take advantage of the fact that ImGuiStorage stores sorted key.
        void SetRange(int n1, int n2, bool v)   { if (n2 < n1) { int tmp = n2; n2 = n1; n1 = tmp; } for (int n = n1; n <= n2; n++) SetSelected(n, v); }
        void SelectAll(int count)               { Storage.Data.resize(count); for (int idx = 0; idx < count; idx++) Storage.Data[idx] = ImGuiStorage::ImGuiStoragePair((ImGuiID)idx, 1); SelectionSize = count; } // This could be using SetRange(), but it this way is faster.

        // FIXME-MULTISELECT: This itself is a good condition we could improve either our API or our demos
        ImGuiMultiSelectData* BeginMultiSelect(ImGuiMultiSelectFlags flags, int items_count)
        {
            ImGuiMultiSelectData* data = ImGui::BeginMultiSelect(flags, (void*)(intptr_t)RangeRef, GetSelected(RangeRef));
            if (data->RequestClear)     { Clear(); }
            if (data->RequestSelectAll) { SelectAll(items_count); }
            return data;
        }
        void EndMultiSelect(int items_count)
        {
            ImGuiMultiSelectData* data = ImGui::EndMultiSelect();
            RangeRef = (int)(intptr_t)data->RangeSrc;
            if (data->RequestClear)     { Clear(); }
            if (data->RequestSelectAll) { SelectAll(items_count); }
            if (data->RequestSetRange)  { SetRange((int)(intptr_t)data->RangeSrc, (int)(intptr_t)data->RangeDst, data->RangeValue ? 1 : 0); }
        }
    };
    struct MultiSelectTestVars
    {
        ExampleSelection    Selection0;
        ExampleSelection    Selection1;
    };
    auto multiselect_guifunc = [](ImGuiTestContext* ctx)
    {
        MultiSelectTestVars& vars = ctx->GetUserData<MultiSelectTestVars>();
        ExampleSelection& selection = vars.Selection0;

        const int ITEMS_COUNT = 100;
        ImGui::Begin("Test Window", NULL, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_AlwaysAutoResize);
        ImGui::Text("(Size = %3d items)", selection.SelectionSize);
        ImGui::Text("(RangeRef = %04d)", selection.RangeRef);
        ImGui::Separator();

        ImGuiMultiSelectData* multi_select_data = selection.BeginMultiSelect(ImGuiMultiSelectFlags_None, ITEMS_COUNT);

        if (ctx->Test->ArgVariant == 1)
            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(ImGui::GetStyle().ItemSpacing.x, 0.0f));

        ImGuiListClipper clipper(ITEMS_COUNT);
        while (clipper.Step())
        {
            if (clipper.DisplayStart > selection.RangeRef)
                multi_select_data->RangeSrcPassedBy = true;
            for (int item_n = clipper.DisplayStart; item_n < clipper.DisplayEnd; item_n++)
            {
                Str64f label("Object %04d", item_n);
                bool item_is_selected = selection.GetSelected(item_n);

                ImGui::SetNextItemSelectionData((void*)(intptr_t)item_n);
                if (ctx->Test->ArgVariant == 0)
                {
                    ImGui::Selectable(label.c_str(), item_is_selected);
                    bool toggled = ImGui::IsItemToggledSelection();
                    if (toggled)
                        selection.SetSelected(item_n, !item_is_selected);
                }
                else if (ctx->Test->ArgVariant == 1)
                {
                    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_SpanAvailWidth;
                    flags |= ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_OpenOnArrow;
                    if (item_is_selected)
                        flags |= ImGuiTreeNodeFlags_Selected;
                    if (ImGui::TreeNodeEx(label.c_str(), flags))
                        ImGui::TreePop();
                    if (ImGui::IsItemToggledSelection())
                        selection.SetSelected(item_n, !item_is_selected);
                }
            }
        }

        if (ctx->Test->ArgVariant == 1)
            ImGui::PopStyleVar();

        selection.EndMultiSelect(ITEMS_COUNT);

        ImGui::End();
    };
    auto multiselect_testfunc = [](ImGuiTestContext* ctx)
    {
        // We are using lots of MouseMove+MouseDown+MouseUp (instead of ItemClick) because we need to test precise MouseUp vs MouseDown reactions.
        ImGuiContext& g = *ctx->UiContext;
        MultiSelectTestVars& vars = ctx->GetUserData<MultiSelectTestVars>();
        ExampleSelection& selection = vars.Selection0;

        ctx->SetRef("Test Window");

        selection.Clear();
        ctx->Yield();
        IM_CHECK_EQ(selection.SelectionSize, 0);

        // Single click
        ctx->ItemClick("Object 0000");
        IM_CHECK_EQ(selection.SelectionSize, 1);
        IM_CHECK_EQ(selection.GetSelected(0), true);

        // Verify that click on another item alter selection on MouseDown
        ctx->MouseMove("Object 0001");
        ctx->MouseDown(0);
        IM_CHECK_EQ(selection.SelectionSize, 1);
        IM_CHECK_EQ(selection.GetSelected(1), true);
        ctx->MouseUp(0);

        // CTRL-A
        ctx->KeyPressMap(ImGuiKey_A, ImGuiKeyModFlags_Ctrl);
        IM_CHECK_EQ(selection.SelectionSize, 100);

        // Verify that click on selected item clear other items from selection on MouseUp
        ctx->MouseMove("Object 0001");
        ctx->MouseDown(0);
        IM_CHECK_EQ(selection.SelectionSize, 100);
        ctx->MouseUp(0);
        IM_CHECK_EQ(selection.SelectionSize, 1);
        IM_CHECK_EQ(selection.GetSelected(1), true);

        // Test SHIFT+Click
        ctx->ItemClick("Object 0001");
        ctx->KeyDownMap(ImGuiKey_COUNT, ImGuiKeyModFlags_Shift);
        ctx->MouseMove("Object 0006");
        ctx->MouseDown(0);
        IM_CHECK_EQ(selection.SelectionSize, 6);
        ctx->MouseUp(0);
        ctx->KeyUpMap(ImGuiKey_COUNT, ImGuiKeyModFlags_Shift);

        // Test that CTRL+A preserve RangeSrc (which was 0001)
        ctx->KeyPressMap(ImGuiKey_A, ImGuiKeyModFlags_Ctrl);
        IM_CHECK_EQ(selection.SelectionSize, 100);
        ctx->KeyDownMap(ImGuiKey_COUNT, ImGuiKeyModFlags_Shift);
        ctx->ItemClick("Object 0008");
        ctx->KeyUpMap(ImGuiKey_COUNT, ImGuiKeyModFlags_Shift);
        IM_CHECK_EQ(selection.SelectionSize, 8);

        // Test reverse clipped SHIFT+Click
        // FIXME-TESTS: ItemInfo query could disable clipper?
        // FIXME-TESTS: We would need to disable clipper because it conveniently rely on cliprect which is affected by actual viewport, so ScrollToBottom() is not enough...
        //ctx->ScrollToBottom();
        ctx->ItemClick("Object 0030");
        ctx->ScrollToTop();
        ctx->KeyDownMap(ImGuiKey_COUNT, ImGuiKeyModFlags_Shift);
        ctx->ItemClick("Object 0002");
        ctx->KeyUpMap(ImGuiKey_COUNT, ImGuiKeyModFlags_Shift);
        IM_CHECK_EQ(selection.SelectionSize, 29);

        // Test ESC to clear selection
        // FIXME-TESTS: Feature not implemented
#if IMGUI_BROKEN_TESTS
        ctx->KeyPressMap(ImGuiKey_Escape);
        ctx->Yield();
        IM_CHECK_EQ(selection.SelectionSize, 0);
#endif

        // Test SHIFT+Arrow
        ctx->ItemClick("Object 0002");
        IM_CHECK_EQ(g.NavId, ctx->GetID("Object 0002"));
        IM_CHECK_EQ(selection.SelectionSize, 1);
        ctx->KeyPressMap(ImGuiKey_DownArrow, ImGuiKeyModFlags_Shift);
        ctx->KeyPressMap(ImGuiKey_DownArrow, ImGuiKeyModFlags_Shift);
        IM_CHECK_EQ(g.NavId, ctx->GetID("Object 0004"));
        IM_CHECK_EQ(selection.SelectionSize, 3);

        // Test CTRL+Arrow
        ctx->KeyPressMap(ImGuiKey_DownArrow, ImGuiKeyModFlags_Ctrl);
        ctx->KeyPressMap(ImGuiKey_DownArrow, ImGuiKeyModFlags_Ctrl);
        IM_CHECK_EQ(g.NavId, ctx->GetID("Object 0006"));
        IM_CHECK_EQ(selection.SelectionSize, 3);

        // Test SHIFT+Arrow after a gap
        ctx->KeyPressMap(ImGuiKey_DownArrow, ImGuiKeyModFlags_Shift);
        IM_CHECK_EQ(g.NavId, ctx->GetID("Object 0007"));
        IM_CHECK_EQ(selection.SelectionSize, 6);

        // Test SHIFT+Arrow reducing selection
        ctx->KeyPressMap(ImGuiKey_UpArrow, ImGuiKeyModFlags_Shift);
        IM_CHECK_EQ(g.NavId, ctx->GetID("Object 0006"));
        IM_CHECK_EQ(selection.SelectionSize, 5);

        // Test CTRL+Shift+Arrow moving or appending without reducing selection
        ctx->KeyPressMap(ImGuiKey_UpArrow, ImGuiKeyModFlags_Ctrl | ImGuiKeyModFlags_Shift, 4);
        IM_CHECK_EQ(g.NavId, ctx->GetID("Object 0002"));
        IM_CHECK_EQ(selection.SelectionSize, 5);

        // Test SHIFT+Arrow replacing selection
        ctx->KeyPressMap(ImGuiKey_UpArrow, ImGuiKeyModFlags_Shift);
        IM_CHECK_EQ(g.NavId, ctx->GetID("Object 0001"));
        IM_CHECK_EQ(selection.SelectionSize, 2);

        // Test Arrow replacing selection
        ctx->KeyPressMap(ImGuiKey_DownArrow);
        IM_CHECK_EQ(g.NavId, ctx->GetID("Object 0002"));
        IM_CHECK_EQ(selection.SelectionSize, 1);
        IM_CHECK_EQ(selection.GetSelected(2), true);

        // Test Home/End
        ctx->KeyPressMap(ImGuiKey_Home);
        IM_CHECK_EQ(g.NavId, ctx->GetID("Object 0000"));
        IM_CHECK_EQ(selection.SelectionSize, 1);
        IM_CHECK_EQ(selection.GetSelected(0), true);
        ctx->KeyPressMap(ImGuiKey_End);
        IM_CHECK_EQ(g.NavId, ctx->GetID("Object 0099"));
        IM_CHECK_EQ(selection.SelectionSize, 1);
        IM_CHECK_EQ(selection.GetSelected(99), true); // Would break if clipped by viewport
        ctx->KeyPressMap(ImGuiKey_Home, ImGuiKeyModFlags_Ctrl);
        IM_CHECK_EQ(g.NavId, ctx->GetID("Object 0000"));
        IM_CHECK_EQ(selection.SelectionSize, 1);
        IM_CHECK_EQ(selection.GetSelected(99), true);
        ctx->KeyPressMap(ImGuiKey_Home);
        IM_CHECK_EQ(selection.SelectionSize, 1);
        IM_CHECK_EQ(selection.GetSelected(99), true); // FIXME: A Home/End/PageUp/PageDown leading to same target doesn't trigger JustMovedTo, may be reasonable.
        ctx->KeyPressMap(ImGuiKey_Space);
        IM_CHECK_EQ(selection.SelectionSize, 1);
        IM_CHECK_EQ(selection.GetSelected(0), true);
        ctx->KeyPressMap(ImGuiKey_End, ImGuiKeyModFlags_Shift);
        IM_CHECK_EQ(g.NavId, ctx->GetID("Object 0099"));
        IM_CHECK_EQ(selection.SelectionSize, 100);
    };

    t = IM_REGISTER_TEST(e, "widgets", "widgets_multiselect_1_selectables");
    t->SetUserDataType<MultiSelectTestVars>();
    t->GuiFunc = multiselect_guifunc;
    t->TestFunc = multiselect_testfunc;
    t->ArgVariant = 0;

    t = IM_REGISTER_TEST(e, "widgets", "widgets_multiselect_2_treenode");
    t->SetUserDataType<MultiSelectTestVars>();
    t->GuiFunc = multiselect_guifunc;
    t->TestFunc = multiselect_testfunc;
    t->ArgVariant = 1;

    t = IM_REGISTER_TEST(e, "widgets", "widgets_multiselect_3_multiple");
    t->SetUserDataType<MultiSelectTestVars>();
    t->GuiFunc = [](ImGuiTestContext* ctx)
    {
        MultiSelectTestVars& vars = ctx->GetUserData<MultiSelectTestVars>();

        const int ITEMS_COUNT = 10;
        ImGui::Begin("Test Window", NULL, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_AlwaysAutoResize);
        for (int scope_n = 0; scope_n < 2; scope_n++)
        {
            ExampleSelection& selection = (scope_n == 1) ? vars.Selection1 : vars.Selection0;
            ImGui::Text("SCOPE %d\n", scope_n);
            ImGui::PushID(Str16f("Scope %d", scope_n).c_str());
            selection.BeginMultiSelect(ImGuiMultiSelectFlags_None, ITEMS_COUNT);
            for (int item_n = 0; item_n < ITEMS_COUNT; item_n++)
            {
                bool item_is_selected = selection.GetSelected(item_n);
                ImGui::SetNextItemSelectionData((void*)(intptr_t)item_n);
                ImGui::Selectable(Str16f("Object %04d", item_n).c_str(), item_is_selected);
                if (ImGui::IsItemToggledSelection())
                    selection.SetSelected(item_n, !item_is_selected);
            }
            selection.EndMultiSelect(ITEMS_COUNT);
            ImGui::PopID();
        }
        ImGui::End();
    };
    t->TestFunc = [](ImGuiTestContext* ctx)
    {
        MultiSelectTestVars& vars = ctx->GetUserData<MultiSelectTestVars>();
        ExampleSelection& selection0 = vars.Selection0;
        ExampleSelection& selection1 = vars.Selection1;

        ctx->SetRef("Test Window");
        selection0.Clear();
        selection1.Clear();

        ctx->ItemClick("Scope 0/Object 0001");
        IM_CHECK_EQ(selection0.SelectionSize, 1);
        IM_CHECK_EQ(selection1.SelectionSize, 0);

        ctx->ItemClick("Scope 1/Object 0002");
        IM_CHECK_EQ(selection0.SelectionSize, 1);
        IM_CHECK_EQ(selection1.SelectionSize, 1);

        ctx->KeyPressMap(ImGuiKey_A, ImGuiKeyModFlags_Ctrl);
        IM_CHECK_EQ(selection0.SelectionSize, 1);
        IM_CHECK_EQ(selection1.SelectionSize, 10);
    };
#endif

    // ## Test Selectable() with ImGuiSelectableFlags_SpanAllColumns inside Columns()
    t = IM_REGISTER_TEST(e, "widgets", "widgets_selectable_span_all_columns");
    t->GuiFunc = [](ImGuiTestContext* ctx)
    {
        ImGui::SetNextWindowSize(ImVec2(300, 200), ImGuiCond_Appearing);
        ImGui::Begin("Test Window", NULL, ImGuiWindowFlags_NoSavedSettings);

        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
        ImGui::Columns(3);
        ImGui::Button("C1");
        ImGui::NextColumn();
        ImGui::Selectable("Selectable", &ctx->GenericVars.Bool1, ImGuiSelectableFlags_SpanAllColumns);
        ctx->GenericVars.Status.QuerySet();
        ImGui::NextColumn();
        ImGui::Button("C3");
        ImGui::Columns();
        ImGui::PopStyleVar();

        ImGui::End();
    };
    t->TestFunc = [](ImGuiTestContext* ctx)
    {
        ctx->SetRef("Test Window");
        ctx->MouseMove("C1", ImGuiTestOpFlags_NoCheckHoveredId); // Button itself won't be hovered, Selectable will!
        IM_CHECK(ctx->GenericVars.Status.Hovered == 1);
        ctx->MouseMove("C3", ImGuiTestOpFlags_NoCheckHoveredId); // Button itself won't be hovered, Selectable will!
        IM_CHECK(ctx->GenericVars.Status.Hovered == 1);
    };

#ifdef IMGUI_HAS_TABLE
    // ## Test ImGuiSelectableFlags_SpanAllColumns flag when used in a table.
    t = IM_REGISTER_TEST(e, "widgets", "widgets_selectable_span_all_table");
    t->GuiFunc = [](ImGuiTestContext* ctx)
    {
        ImGui::SetNextWindowSize(ImVec2(300, 200), ImGuiCond_Appearing);
        ImGui::Begin("Test Window", NULL, ImGuiWindowFlags_NoSavedSettings);

        const int column_count = 3;
        ImGui::BeginTable("table", column_count, ImGuiTableFlags_NoSavedSettings | ImGuiTableFlags_Reorderable);
        for (int i = 0; i < column_count; i++)
            ImGui::TableSetupColumn(Str30f("%d", i + 1).c_str());

        ImGui::TableHeadersRow();
        ImGui::TableNextRow();

        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
        ImGui::TableSetColumnIndex(0);
        ImGui::Button("C1");
        ImGui::TableSetColumnIndex(1);
        ImGui::Selectable("Selectable", &ctx->GenericVars.Bool1, ImGuiSelectableFlags_SpanAllColumns);
        ctx->GenericVars.Status.QuerySet();
        ImGui::TableSetColumnIndex(2);
        ImGui::Button("C3");
        ImGui::PopStyleVar();

        ImGui::EndTable();

        ImGui::End();
    };
    t->TestFunc = [](ImGuiTestContext* ctx)
    {
        ctx->SetRef("Test Window");
        ImGuiTable* table = ImGui::TableFindByID(ctx->GetID("table"));

        for (int i = 0; i < 2; i++)
        {
            ctx->MouseMove("table/C1", ImGuiTestOpFlags_NoCheckHoveredId); // Button itself won't be hovered, Selectable will!
            IM_CHECK(ctx->GenericVars.Status.Hovered == 1);
            ctx->MouseMove("table/C3", ImGuiTestOpFlags_NoCheckHoveredId); // Button itself won't be hovered, Selectable will!
            IM_CHECK(ctx->GenericVars.Status.Hovered == 1);

            // Reorder columns and test again
            ctx->ItemDragAndDrop(TableGetHeaderID(table, "1"), TableGetHeaderID(table, "2"));
        }
    };
#endif

    // ## Test sliders with inverted ranges.
    t = IM_REGISTER_TEST(e, "widgets", "widgets_slider_ranges");
    struct SliderRangeVars { struct { ImGuiDataTypeStorage Cur, Min, Max; int ResultFlags; } Values[ImGuiDataType_COUNT]; SliderRangeVars() { memset(this, 0, sizeof(*this)); } };
    t->SetUserDataType<SliderRangeVars>();
    t->GuiFunc = [](ImGuiTestContext* ctx)
    {
        SliderRangeVars& vars = ctx->GetUserData<SliderRangeVars>();
        const int OUT_OF_RANGE_FLAG = 1;
        const int MID_OF_RANGE_FLAG = 2;

        // Submit sliders for each data type, with and without inverted range
        ImGui::Begin("Test Window", NULL, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_AlwaysAutoResize);
        for (ImGuiDataType data_type = 0; data_type < ImGuiDataType_COUNT; data_type++)
        {
            const ImGuiDataTypeInfo* data_type_info = ImGui::DataTypeGetInfo(data_type);
            for (int invert_range = 0; invert_range < 2; invert_range++)
            {
                auto& v = vars.Values[data_type];
                GetSliderTestRanges(data_type, &v.Min, &v.Max);
                Str30f label("%s%c", data_type_info->Name, invert_range == 0 ? '+' : '-');
                if (invert_range)
                    ImGui::SliderScalar(label.c_str(), data_type, &v.Cur, &v.Max, &v.Min);
                else
                    ImGui::SliderScalar(label.c_str(), data_type, &v.Cur, &v.Min, &v.Max);

                v.ResultFlags = 0;
                if (ImGui::DataTypeCompare(data_type, &v.Cur, &v.Min) < 0 || ImGui::DataTypeCompare(data_type, &v.Cur, &v.Max) > 0)
                    v.ResultFlags |= OUT_OF_RANGE_FLAG;
                if (ImGui::DataTypeCompare(data_type, &v.Cur, &v.Min) > 0 && ImGui::DataTypeCompare(data_type, &v.Cur, &v.Max) < 0)
                    v.ResultFlags |= MID_OF_RANGE_FLAG;
                ImGui::SameLine();
                ImGui::Text("R:%d", v.ResultFlags); // Doesn't make sense to display if not reset
            }
        }
        ImGui::End();
    };
    t->TestFunc = [](ImGuiTestContext* ctx)
    {
        SliderRangeVars& vars = ctx->GetUserData<SliderRangeVars>();
        const int OUT_OF_RANGE_FLAG = 1;
        const int MID_OF_RANGE_FLAG = 2;

        ctx->SetRef("Test Window");
        for (int data_type = 0; data_type < ImGuiDataType_COUNT; data_type++)
        {
            for (int invert_range = 0; invert_range < 2; invert_range++)
            {
                auto& v = vars.Values[data_type];
                ImGuiDataTypeStorage v_left = invert_range ? v.Max : v.Min;
                ImGuiDataTypeStorage v_right = invert_range ? v.Min : v.Max;

                const ImGuiDataTypeInfo* data_type_info = ImGui::DataTypeGetInfo(data_type);
                Str30f label("%s%c", data_type_info->Name, invert_range == 0 ? '+' : '-');

                for (int side = 0; side < 2; side++)
                {
                    // Click on center
                    ctx->MouseMove(label.c_str());
                    ctx->MouseDown();
                    IM_CHECK((v.ResultFlags & OUT_OF_RANGE_FLAG) == 0); // Widget value does not fall out of range
                    IM_CHECK((v.ResultFlags & MID_OF_RANGE_FLAG) != 0); // Widget value is between min and max

                    // Drag to left/right edge
                    ctx->MouseMove(label.c_str(), side ? ImGuiTestOpFlags_MoveToEdgeR : ImGuiTestOpFlags_MoveToEdgeL);
                    ctx->MouseUp();

                    // Print values and compare
                    char buf0[32], buf1[32], buf2[32];
                    ImGui::DataTypeFormatString(buf0, 32, data_type, &v.Cur, data_type_info->PrintFmt);
                    ImGui::DataTypeFormatString(buf1, 32, data_type, &v.Min, data_type_info->PrintFmt);
                    ImGui::DataTypeFormatString(buf2, 32, data_type, &v.Max, data_type_info->PrintFmt);
                    ctx->LogInfo("## DataType: %s, Inverted: %d, cur = %s, min = %s, max = %s", data_type_info->Name, invert_range, buf0, buf1, buf2);
                    IM_CHECK((v.ResultFlags & OUT_OF_RANGE_FLAG) == 0); // Widget value does not fall out of range
                    IM_CHECK((v.ResultFlags & MID_OF_RANGE_FLAG) == 0); // Widget value is not between min and max
                    if (side == 0)
                        IM_CHECK(ImGui::DataTypeCompare(data_type, &v.Cur, &v_left) == 0);
                    else
                        IM_CHECK(ImGui::DataTypeCompare(data_type, &v.Cur, &v_right) == 0);
                }
            }
        }

        // Test case where slider knob position calculation would snap to either end of the widget in some cases.
        // Breaks linking with GCC Release mode as those functions in imgui_widgets.cpp gets inlined.
#if 0
        const float s8_ratio_half = ImGui::ScaleRatioFromValueT<ImS32, ImS32, float>(ImGuiDataType_S8, 0, INT8_MIN, INT8_MAX, false, false, false);
        const float u8_ratio_half = ImGui::ScaleRatioFromValueT<ImU32, ImS32, float>(ImGuiDataType_U8, UINT8_MAX / 2, 0, UINT8_MAX, false, false, false);
        const float s8_ratio_half_inv = ImGui::ScaleRatioFromValueT<ImS32, ImS32, float>(ImGuiDataType_S8, 0, INT8_MAX, INT8_MIN, false, false, false);
        const float u8_ratio_half_inv = ImGui::ScaleRatioFromValueT<ImU32, ImS32, float>(ImGuiDataType_U8, UINT8_MAX / 2, UINT8_MAX, 0, false, false, false);
        IM_CHECK(ImAbs(s8_ratio_half - 0.5f) < 0.01f);
        IM_CHECK(ImAbs(u8_ratio_half - 0.5f) < 0.01f);
        IM_CHECK(ImAbs(s8_ratio_half_inv - 0.5f) < 0.01f);
        IM_CHECK(ImAbs(u8_ratio_half_inv - 0.5f) < 0.01f);
#endif
    };

    // ## Test logarithmic slider
    t = IM_REGISTER_TEST(e, "widgets", "widgets_slider_logarithmic");
    t->GuiFunc = [](ImGuiTestContext* ctx)
    {
        ImGui::Begin("Test Window", NULL, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_AlwaysAutoResize);
        ImGui::SetNextItemWidth(400);
        ImGui::SliderFloat("slider", &ctx->GenericVars.Float1, -10.0f, 10.0f, "%.2f", ImGuiSliderFlags_Logarithmic);
        ImGui::End();
    };
    t->TestFunc = [](ImGuiTestContext* ctx)
    {
        ctx->SetRef("Test Window");
        ctx->ItemClick("slider", 0);
        ctx->Yield();
        IM_CHECK_EQ(ctx->GenericVars.Float1, 0.f);

        ctx->ItemClick("slider", 0, ImGuiTestOpFlags_MoveToEdgeR);
        ctx->Yield();
        IM_CHECK_EQ(ctx->GenericVars.Float1, 10.f);

        ctx->ItemClick("slider", 0, ImGuiTestOpFlags_MoveToEdgeL);
        ctx->Yield();
        IM_CHECK_EQ(ctx->GenericVars.Float1, -10.f);

        // Drag a bit
        ctx->ItemClick("slider", 0);
        float x_offset[] = {50.f,   100.f,  150.f,  190.f};
        float slider_v[] = {0.06f,  0.35f,  2.11f,  8.97f};
        for (float sign : {-1.f, 1.f})
            for (int i = 0; i < IM_ARRAYSIZE(x_offset); i++)
            {
                ctx->ItemDragWithDelta("slider", ImVec2(sign * x_offset[i], 0.f));
                IM_CHECK_EQ(ctx->GenericVars.Float1, sign * slider_v[i]);
            }
    };

    // ## Test slider navigation path using keyboard.
    t = IM_REGISTER_TEST(e, "widgets", "widgets_slider_nav");
    t->GuiFunc = [](ImGuiTestContext* ctx)
    {
        ImGui::Begin("Test Window", NULL, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_AlwaysAutoResize);
        ImGui::SliderFloat("Slider 1", &ctx->GenericVars.Float1, -100.0f, 100.0f, "%.2f");
        ImGui::SliderInt("Slider 2", &ctx->GenericVars.Int1, 0, 100);
        ImGui::End();
    };
    t->TestFunc = [](ImGuiTestContext* ctx)
    {
        ImGuiContext& g = *ImGui::GetCurrentContext();
        ctx->SetRef("Test Window");

        ctx->ItemNavActivate("Slider 1");
        IM_CHECK_EQ(ctx->GenericVars.Float1, 0.f);
        IM_CHECK_EQ(ctx->GenericVars.Int1, 0);
        IM_CHECK_EQ(g.NavId, ctx->GetID("Slider 1"));

        // FIXME-TESTS: the step/slow value are technically not exposed and bounds to change. Perhaps this test should only compare the respective magnitude of changes.
        const float slider_float_step = 2.0f;
        const float slider_float_step_slow = 0.2f;
        float slider_float_value = ctx->GenericVars.Float1;
        ctx->NavKeyPress(ImGuiNavInput_DpadLeft);
        IM_CHECK_EQ(ctx->GenericVars.Float1, slider_float_value - slider_float_step);
        ctx->NavKeyDown(ImGuiNavInput_TweakSlow);
        ctx->NavKeyPress(ImGuiNavInput_DpadRight);
        ctx->NavKeyUp(ImGuiNavInput_TweakSlow);
        IM_CHECK_EQ(ctx->GenericVars.Float1, slider_float_value - slider_float_step + slider_float_step_slow);

        ctx->NavKeyPress(ImGuiNavInput_DpadDown);
        ctx->NavKeyPress(ImGuiNavInput_Activate);
        IM_CHECK_EQ(g.NavId, ctx->GetID("Slider 2"));;

        const int slider_int_step = 1;
        const int slider_int_step_slow = 1;
        int slider_int_value = ctx->GenericVars.Int1;
        ctx->NavKeyPress(ImGuiNavInput_DpadRight);
        IM_CHECK_EQ(ctx->GenericVars.Int1, slider_float_value + slider_int_step);
        ctx->NavKeyDown(ImGuiNavInput_TweakSlow);
        ctx->NavKeyPress(ImGuiNavInput_DpadLeft);
        ctx->NavKeyUp(ImGuiNavInput_TweakSlow);
        IM_CHECK_EQ(ctx->GenericVars.Int1, slider_int_value + slider_int_step - slider_int_step_slow);

        ctx->NavKeyPress(ImGuiNavInput_DpadUp);
        IM_CHECK_EQ(g.NavId, ctx->GetID("Slider 1"));
    };

    // ## Test whether numbers after format specifier do not influence widget value.
    t = IM_REGISTER_TEST(e, "widgets", "widgets_slider_format");
    t->GuiFunc = [](ImGuiTestContext* ctx)
    {
        ImGui::Begin("Test Window", NULL, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_AlwaysAutoResize);
        ImGui::SliderInt("Int", &ctx->GenericVars.Int1, 10, 100, "22%d00");
        ImGui::SliderInt("Int2", &ctx->GenericVars.Int2, 10, 100, "22%'d"); // May not display correct with all printf implementation, but our test stands
        ImGui::SliderFloat("Float", &ctx->GenericVars.Float1, 10.0f, 100.0f, "22%.0f00");
        ImGui::End();
    };
    t->TestFunc = [](ImGuiTestContext* ctx)
    {
        ctx->SetRef("Test Window");
        ctx->ItemClick("Int", 0, ImGuiTestOpFlags_MoveToEdgeR);
        IM_CHECK_EQ(ctx->GenericVars.Int1, 100);
        ctx->ItemClick("Int2", 0, ImGuiTestOpFlags_MoveToEdgeR);
        IM_CHECK_EQ(ctx->GenericVars.Int2, 100);
        ctx->ItemClick("Float", 0, ImGuiTestOpFlags_MoveToEdgeR);
        IM_CHECK_EQ(ctx->GenericVars.Float1, 100.0f);
    };

    // ## Test tooltip positioning in various conditions.
    t = IM_REGISTER_TEST(e, "widgets", "widgets_popup_positioning");
    struct TooltipPosVars { ImVec2 Size = ImVec2(50, 50); };
    t->SetUserDataType<TooltipPosVars>();
    t->GuiFunc = [](ImGuiTestContext* ctx)
    {
        TooltipPosVars& vars = ctx->GetUserData<TooltipPosVars>();

        auto popup_debug_controls = [&]()
        {
            if (ctx->RunFlags & ImGuiTestRunFlags_GuiFuncOnly)
            {
                float step = ctx->UiContext->IO.DeltaTime * 500.0f;
                if (ImGui::IsKeyDown(ImGui::GetKeyIndex(ImGuiKey_UpArrow))) vars.Size.y -= step;
                if (ImGui::IsKeyDown(ImGui::GetKeyIndex(ImGuiKey_DownArrow))) vars.Size.y += step;
                if (ImGui::IsKeyDown(ImGui::GetKeyIndex(ImGuiKey_LeftArrow))) vars.Size.x -= step;
                if (ImGui::IsKeyDown(ImGui::GetKeyIndex(ImGuiKey_RightArrow))) vars.Size.x += step;
            }
        };

        ImGui::Begin("Test Window", NULL, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoNav);
        if (ctx->RunFlags & ImGuiTestRunFlags_GuiFuncOnly)
            ImGui::DragFloat2("Tooltip Size", &vars.Size.x);
        ImGui::Button("Tooltip", ImVec2(100, 0));
        if (ImGui::IsItemHovered())
        {
            ImGui::BeginTooltip();
            ImGui::InvisibleButton("Space", vars.Size);
            popup_debug_controls();
            ImGui::EndTooltip();
        }
        if (ImGui::Button("Popup", ImVec2(100, 0)))
            ImGui::OpenPopup("Popup");
        if (ImGui::BeginPopup("Popup"))
        {
            ImGui::InvisibleButton("Space", vars.Size);
            popup_debug_controls();
            ImGui::EndPopup();
        }
        ImGui::End();
    };
    t->TestFunc = [](ImGuiTestContext* ctx)
    {
        ImGuiContext& g = *ctx->UiContext;
        TooltipPosVars& vars = ctx->GetUserData<TooltipPosVars>();
        ImVec2 viewport_pos = ImGui::GetMainViewport()->Pos;
        ImVec2 viewport_size = ImGui::GetMainViewport()->Size;

        struct TestCase
        {
            ImVec2 Pos;             // Window position
            ImVec2 Pivot;           // Window position pivot
            ImGuiDir DirSmall;      // Expected default tooltip location
            ImGuiDir DirBigH;       // Expected location when tooltip is as wide as viewport
            ImGuiDir DirBigV;       // Expected location when tooltip is as high as viewport
        };

        // Test tooltip positioning around viewport corners
        static TestCase test_cases[] =
        {
            // [0] Top-left corner
            {
                viewport_pos,
                ImVec2(0.0f, 0.0f),
                ImGuiDir_Right,
                ImGuiDir_Down,
                ImGuiDir_Right,
            },
            // [1] Top edge
            {
                viewport_pos + ImVec2(viewport_size.x * 0.5f, 0.0f),
                ImVec2(0.5f, 0.0f),
                ImGuiDir_Right,
                ImGuiDir_Down,
                ImGuiDir_Right,
            },
            // [2] Top-right corner
            {
                viewport_pos + ImVec2(viewport_size.x, 0.0f),
                ImVec2(1.0f, 0.0f),
                ImGuiDir_Down,
                ImGuiDir_Down,
                ImGuiDir_Left,
            },
            // [3] Right edge
            {
                viewport_pos + ImVec2(viewport_size.x, viewport_size.y * 0.5f),
                ImVec2(1.0f, 0.5f),
                ImGuiDir_Down,
                ImGuiDir_Down,
                ImGuiDir_Left,
            },
            // [4] Bottom-right corner
            {
                viewport_pos + viewport_size,
                ImVec2(1.0f, 1.0f),
                ImGuiDir_Up,
                ImGuiDir_Up,
                ImGuiDir_Left,
            },
            // [5] Bottom edge
            {
                viewport_pos + ImVec2(viewport_size.x * 0.5f, viewport_size.y),
                ImVec2(0.5f, 1.0f),
                ImGuiDir_Right,
                ImGuiDir_Up,
                ImGuiDir_Right,
            },
            // [6] Bottom-left corner
            {
                viewport_pos + ImVec2(0.0f, viewport_size.y),
                ImVec2(0.0f, 1.0f),
                ImGuiDir_Right,
                ImGuiDir_Up,
                ImGuiDir_Right,
            },
            // [7] Left edge
            {
                viewport_pos + ImVec2(0.0f, viewport_size.y * 0.5f),
                ImVec2(0.0f, 0.5f),
                ImGuiDir_Right,
                ImGuiDir_Down,
                ImGuiDir_Right,
            },
        };

        ctx->SetRef("Test Window");

        for (int variant = 0; variant < 2; variant++)
        {
            const char* button_name = variant ? "Popup" : "Tooltip";
            ctx->LogInfo("## Test variant: %s", button_name);
            ctx->ItemClick(button_name);        // Force tooltip creation so we can grab the pointer
            ImGuiWindow* tooltip = variant ? g.NavWindow : ctx->GetWindowByRef("##Tooltip_00");

            for (auto& test_case : test_cases)
            {
                ctx->LogInfo("## Test case %d", (int)(&test_case - test_cases));
                vars.Size = ImVec2(50, 50);
                ctx->WindowMove(ctx->RefID, test_case.Pos, test_case.Pivot);
                ctx->ItemClick(button_name);

                // Check default tooltip location
                IM_CHECK_EQ(g.HoveredIdPreviousFrame, ctx->GetID(button_name));
                IM_CHECK_EQ(tooltip->AutoPosLastDirection, test_case.DirSmall);

                // Check tooltip location when it is real wide and verify that location does not change once it becomes too wide
                // First iteration: tooltip is just wide enough to fit within viewport
                // First iteration: tooltip is wider than viewport
                for (int j = 0; j < 2; j++)
                {
                    vars.Size = ImVec2((j * 0.25f * viewport_size.x) + (viewport_size.x - (g.Style.WindowPadding.x + g.Style.DisplaySafeAreaPadding.x) * 2), 50);
                    ctx->ItemClick(button_name);
                    IM_CHECK(tooltip->AutoPosLastDirection == test_case.DirBigH);
                }

                // Check tooltip location when it is real tall and verify that location does not change once it becomes too tall
                // First iteration: tooltip is just tall enough to fit within viewport
                // First iteration: tooltip is taller than viewport
                for (int j = 0; j < 2; j++)
                {
                    vars.Size = ImVec2(50, (j * 0.25f * viewport_size.x) + (viewport_size.y - (g.Style.WindowPadding.y + g.Style.DisplaySafeAreaPadding.y) * 2));
                    ctx->ItemClick(button_name);
                    IM_CHECK(tooltip->AutoPosLastDirection == test_case.DirBigV);
                }

                IM_CHECK_EQ(g.HoveredIdPreviousFrame, ctx->GetID(button_name));
            }
        }
    };

    // ## Test drag & drop using three main mouse buttons.
    t = IM_REGISTER_TEST(e, "widgets", "widgets_drag_mouse_buttons");
    struct DragMouseButtonsVars { bool Pressed = false; bool Dropped = false; };
    t->SetUserDataType<DragMouseButtonsVars>();
    t->GuiFunc = [](ImGuiTestContext* ctx)
    {
        ImGuiContext& g = *ImGui::GetCurrentContext();
        DragMouseButtonsVars& vars = ctx->GetUserData<DragMouseButtonsVars>();
        ImGui::SetNextWindowSize(ImVec2(0, 0), ImGuiCond_Appearing);
        if (ImGui::Begin("Window", NULL, ImGuiWindowFlags_NoSavedSettings))
        {
            vars.Pressed |= ImGui::Button("Button");

            // This is a workaround for button widget not reacting to mouse clicks other than the left one.
            // See https://github.com/ocornut/imgui/issues/3885 for more details.
            ImGui::ButtonBehavior(g.LastItemData.Rect, g.LastItemData.ID, NULL, NULL, ImGuiButtonFlags_MouseButtonLeft | ImGuiButtonFlags_MouseButtonMiddle | ImGuiButtonFlags_MouseButtonRight);

            if (ImGui::BeginDragDropSource())
            {
                for (int button = 0; button < ImGuiMouseButton_COUNT; button++)
                    if (ImGui::IsMouseDown(button))
                        ImGui::Text("Dragged by button %d", button);
                ImGui::SetDragDropPayload("Button", "Works", 6);
                ImGui::EndDragDropSource();
            }

            ImGui::Button("Drop Here");
            if (ImGui::BeginDragDropTarget())
            {
                if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("Button"))
                    vars.Dropped = payload->Data != NULL && strcmp((const char*)payload->Data, "Works") == 0;
                ImGui::EndDragDropTarget();
            }
        }
        ImGui::End();
    };
    t->TestFunc = [](ImGuiTestContext* ctx)
    {
        DragMouseButtonsVars& vars = ctx->GetUserData<DragMouseButtonsVars>();
        ctx->SetRef("Window");

        // Clicking still works.
        for (int button = 0; button < 3; button++)
        {
            vars.Pressed = false;
            ctx->ItemClick("Button", button);
            IM_CHECK(vars.Pressed);
        }

        // Drag & drop using all mouse buttons work.
        // FIXME: At this time only left, right and middle mouse buttons are supported for this usecase.
        for (ImGuiMouseButton button = 0; button < 3; button++)
        {
            vars.Dropped = false;
            ctx->ItemDragAndDrop("Button", "Drop Here", button);
            IM_CHECK(vars.Dropped);
        }
    };

    // ## Test disabled items setting g.HoveredId and taking clicks.
#if (IMGUI_VERSION_NUM >= 18310)
    t = IM_REGISTER_TEST(e, "widgets", "widgets_disabled");
    struct WidgetsDisabledVars
    {
        bool WidgetsDisabled;
        bool Activated[4];
        bool Hovered[4];
        bool HoveredDisabled[4];
        void Reset() { memset(this, 0, sizeof(*this)); }
        WidgetsDisabledVars() { Reset(); }
    };
    t->SetUserDataType<WidgetsDisabledVars>();
    t->GuiFunc = [](ImGuiTestContext* ctx)
    {
        WidgetsDisabledVars& vars = ctx->GetUserData<WidgetsDisabledVars>();
        ImGui::Begin("Test Window", NULL, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_MenuBar);
        if (ImGui::BeginMenuBar())
        {
            if (ImGui::BeginMenu("Menu 1"))
            {
                if (ImGui::BeginMenu("Menu Enabled"))
                {
                    ImGui::MenuItem("Item");
                    ImGui::EndMenu();
                }
                if (ImGui::BeginMenu("Menu Disabled", false))
                {
                    ImGui::MenuItem("Item");
                    ImGui::EndMenu();
                }
                ImGui::EndMenu();
            }
            ImGui::EndMenuBar();
        }

        ImGui::Selectable("Enabled A");
        int index = 0;

        if (vars.WidgetsDisabled)
            ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);

        vars.Activated[index] |= ImGui::Button("Button");
        vars.Hovered[index] |= ImGui::IsItemHovered();
        vars.HoveredDisabled[index] |= ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled);
        if (ctx->FrameCount == 0)
            IM_CHECK_EQ(ImGui::GetItemID(), ImGui::GetID("Button"));      // Make sure widget has an ID
        index++;

        float f = 0.0f;
        vars.Activated[index] |= ImGui::DragFloat("DragFloat", &f);
        vars.Hovered[index] |= ImGui::IsItemHovered();
        vars.HoveredDisabled[index] |= ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled);
        if (ctx->FrameCount == 0)
            IM_CHECK_EQ(ImGui::GetItemID(), ImGui::GetID("DragFloat"));     // Make sure widget has an ID
        index++;

        vars.Activated[index] |= ImGui::Selectable("Selectable", false, 0);
        vars.Hovered[index] |= ImGui::IsItemHovered();
        vars.HoveredDisabled[index] |= ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled);
        if (ctx->FrameCount == 0)
            IM_CHECK_EQ(ImGui::GetItemID(), ImGui::GetID("Selectable"));    // Make sure widget has an ID
        index++;

        if (vars.WidgetsDisabled)
            ImGui::PopItemFlag();

        vars.Activated[index] |= ImGui::Selectable("SelectableFlag", false, vars.WidgetsDisabled ? ImGuiSelectableFlags_Disabled : 0);
        vars.Hovered[index] |= ImGui::IsItemHovered();
        vars.HoveredDisabled[index] |= ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled);
        if (ctx->FrameCount == 0)
            IM_CHECK_EQ(ImGui::GetItemID(), ImGui::GetID("SelectableFlag"));    // Make sure widget has an ID
        index++;

        ImGui::Selectable("Enabled B");

        ImGui::End();
    };
    t->TestFunc = [](ImGuiTestContext* ctx)
    {
        ImGuiContext& g = *ctx->UiContext;
        WidgetsDisabledVars& vars = ctx->GetUserData<WidgetsDisabledVars>();
        const char* disabled_items[] = { "Button", "DragFloat", "Selectable", "SelectableFlag" }; // Matches order of arrays in WidgetsDisabledVars.

        ImGuiWindow* window = ctx->GetWindowByRef("Test Window");
        ctx->SetRef("Test Window");
        vars.WidgetsDisabled = true;

        // Navigating over disabled menu.
        ctx->MenuAction(ImGuiTestAction_Hover, "Menu 1/Menu Enabled/Item");
        IM_CHECK(g.NavWindow != NULL);
        IM_CHECK_STR_EQ(g.NavWindow->Name, "##Menu_01");
        ctx->MenuAction(ImGuiTestAction_Hover, "Menu 1/Menu Disabled");
        IM_CHECK(g.NavWindow != NULL);
        IM_CHECK_STR_EQ(g.NavWindow->Name, "##Menu_00");

        // Navigating over a disabled item.
        ctx->ItemClick("Enabled A");
        IM_CHECK_EQ(g.NavId, ctx->GetID("Enabled A"));
        ctx->KeyPressMap(ImGuiKey_DownArrow);
        IM_CHECK_EQ(g.NavId, ctx->GetID("Enabled B"));              // Make sure we have skipped disabled items.
        ctx->KeyPressMap(ImGuiKey_Escape);

        // Clicking a disabled item.
        for (int i = 0; i < IM_ARRAYSIZE(disabled_items); i++)
        {
            ctx->MouseMove(disabled_items[i]);
            vars.Reset();
            vars.WidgetsDisabled = true;
            ctx->ItemClick(disabled_items[i]);
            IM_CHECK(vars.Activated[i] == false);                   // Was not clicked because button is disabled.
            IM_CHECK(vars.Hovered[i] == false);                     // Wont report as being hovered because button is disabled.
            IM_CHECK(vars.HoveredDisabled[i] == true);              // IsItemHovered with ImGuiHoveredFlags_AllowWhenDisabled.
            IM_CHECK(g.HoveredId == ctx->GetID(disabled_items[i])); // Will set HoveredId even when disabled.
        }

        // Dragging a disabled item.
        ImVec2 window_pos = window->Pos;
        for (int i = 0; i < IM_ARRAYSIZE(disabled_items); i++)
        {
            ctx->MouseMove(disabled_items[i]);
            vars.Reset();
            vars.WidgetsDisabled = true;
            ctx->ItemDragWithDelta(disabled_items[i], ImVec2(30, 0));
            IM_CHECK(window_pos == window->Pos);                    // Disabled items consume click events.
        }

        // Disable active ID when widget gets disabled while held.
        for (int i = 0; i < IM_ARRAYSIZE(disabled_items); i++)
        {
            vars.WidgetsDisabled = false;
            ctx->MouseMove(disabled_items[i]);
            ctx->MouseDown();
            IM_CHECK(g.ActiveId == ctx->GetID(disabled_items[i]));  // Enabled item is active.
            vars.WidgetsDisabled = true;
            ctx->Yield();
            IM_CHECK(g.ActiveId == 0);                              // Disabling item while it is active deactivates it.
            ctx->MouseUp();
        }
    };
#endif

    // ## Test BeginDisabled()/EndDisabled()
#if (IMGUI_VERSION_NUM >= 18405)
    t = IM_REGISTER_TEST(e, "widgets", "widgets_disabled_2");
    struct BeginDisabledVars
    {
        struct BeginDisabledItemInfo
        {
            const char* Name = NULL;
            ImGuiTestGenericStatus Status = {};
            ImGuiItemFlags FlagsBegin = 0;
            ImGuiItemFlags FlagsEnd = 0;
            float AlphaBegin = 0;
            float AlphaEnd = 0;

            BeginDisabledItemInfo(const char* name) { Name = name; }
        };
        BeginDisabledItemInfo ButtonInfo[6] = { "A", "B", "C", "D", "E", "F" };
    };
    t->SetUserDataType<BeginDisabledVars>();
    t->GuiFunc = [](ImGuiTestContext* ctx)
    {
        ImGuiContext& g = *ctx->UiContext;
        BeginDisabledVars& vars = ctx->GetUserData<BeginDisabledVars>();
        ImGui::Begin("Test Window", NULL, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_MenuBar);

        int index = 0;
        auto begin_disabled = [&](bool disabled = true)
        {
            auto& button_info = vars.ButtonInfo[index];
            button_info.FlagsBegin = g.CurrentItemFlags;
            button_info.AlphaBegin = g.DisabledAlphaBackup;
            ImGui::BeginDisabled(disabled);
            bool flag_enabled = (index % 2) == 0;
            ImGui::PushItemFlag(ImGuiItemFlags_NoNav, flag_enabled);    // Add a random flag to try mix things up.
            index++;
        };
        auto end_disabled = [&]()
        {
            index--;
            auto& button_info = vars.ButtonInfo[index];
            ImGui::PopItemFlag();                                       // ImGuiItemFlags_NoNav
            ImGui::EndDisabled();
            button_info.FlagsEnd = g.CurrentItemFlags;
            button_info.AlphaEnd = g.DisabledAlphaBackup;
        };

        begin_disabled();
        vars.ButtonInfo[index].Status.QueryInc(ImGui::Button("A"));
        begin_disabled();
        vars.ButtonInfo[index].Status.QueryInc(ImGui::Button("B"));
        end_disabled();
        begin_disabled(false);
        vars.ButtonInfo[index].Status.QueryInc(ImGui::Button("C"));
        end_disabled();
        vars.ButtonInfo[index].Status.QueryInc(ImGui::Button("D"));
        end_disabled();

        begin_disabled();
        bool ret = ImGui::Button("E");
        end_disabled();
        vars.ButtonInfo[4].Status.QueryInc(ret);

        ImGui::BeginDisabled(false);
        vars.ButtonInfo[5].Status.QueryInc(ImGui::Button("F"));
        ImGui::EndDisabled();
        ImGui::End();
    };
    t->TestFunc = [](ImGuiTestContext* ctx)
    {
        ImGuiContext& g = *ctx->UiContext;
        BeginDisabledVars& vars = ctx->GetUserData<BeginDisabledVars>();
        ctx->SetRef("Test Window");
        for (int i = 0; i < 5; i++)
        {
            auto& button_info = vars.ButtonInfo[i];
            ctx->LogDebug("Button %s", button_info.Name);
            ctx->ItemClick(button_info.Name);
            IM_CHECK(button_info.Status.Ret == 0);                      // No clicks
            IM_CHECK(button_info.Status.Clicked == 0);
            IM_CHECK(g.HoveredId == ctx->GetID(button_info.Name));      // HoveredId is set
            IM_CHECK(button_info.FlagsBegin == button_info.FlagsEnd);   // Flags and Alpha match between Begin/End calls
            IM_CHECK(button_info.AlphaBegin == button_info.AlphaEnd);
        }
        ctx->ItemClick("E");
        IM_CHECK(vars.ButtonInfo[4].Status.Hovered == 0);               // Ensure we rely on last item storage, not current state
        ctx->ItemClick("F");
        IM_CHECK(vars.ButtonInfo[5].Status.Ret == 1);                   // BeginDisabled(false) does not prevent clicks
        IM_CHECK(vars.ButtonInfo[5].Status.Clicked == 1);
    };
#endif
}
