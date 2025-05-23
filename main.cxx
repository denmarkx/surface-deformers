#include "windowFramework.h"
#include "pandaFramework.h"
#include "directionalLight.h"

#include "deformers/sineDeformer.h"
#include "deformers/twistDeformer.h"
#include "deformers/bendDeformer.h"
#include "deformers/expandDeformer.h"
#include "imgui/panda3d_imgui_main.cxx"

typedef SineDeformer TYPE_DEFORMER;

TYPE_DEFORMER* deformer;

static AsyncTask::DoneStatus do_task(GenericAsyncTask* task, void* data) {
    TYPE_DEFORMER* deformer = (TYPE_DEFORMER*)data;
    deformer->deform_all(0); // task->get_elapsed_time());
    return AsyncTask::DS_cont;
}

void create_axis_button(Axis &axis, const char* name, int imgui_id) {
    int *axis_ptr = reinterpret_cast<int*>(&axis);

    ImGui::PushID(imgui_id);
    ImGui::Text(name);
    ImGui::RadioButton("X", axis_ptr, 0);
    ImGui::SameLine();
    ImGui::RadioButton("Y", axis_ptr, 1);
    ImGui::SameLine();
    ImGui::RadioButton("Z", axis_ptr, 2);
    ImGui::PopID();

    axis = static_cast<Axis>(*axis_ptr);
}

static void render_frame() {
    ImGui::SetNextWindowContentSize(ImVec2(250, 0.0));
    ImGui::Begin("Deformer Options", NULL, ImGuiWindowFlags_AlwaysAutoResize);
    auto func_map = deformer->options.func_map;

    std::string name;
    float* var_ptr = nullptr;
    double min, max;
    int i = 0;

    // Function Options (see options.func_map):
    for (auto it = func_map.begin(); it != func_map.end(); it++) {
        var_ptr = it->second.first;
        min = it->second.second[0];
        max = it->second.second[1];
        ImGui::PushID(i);
        ImGui::Text(it->first.c_str());
        ImGui::SliderFloat("", var_ptr, min, max, "%.3f", ImGuiSliderFlags_NoInput);
        ImGui::PopID();
        i++;
    }

    Axis original_major = deformer->axis;
    create_axis_button(deformer->axis, "Major Axis", 0);
    create_axis_button(deformer->_minor_axis_a, "Minor Axis A", 1);
    create_axis_button(deformer->_minor_axis_b, "Minor Axis B", 2);

    static bool auto_manage_minor_axes = true;
    ImGui::Checkbox("Manage Minor Axes Automatically?", &auto_manage_minor_axes);

    if (auto_manage_minor_axes && (original_major != deformer->axis)) {
        deformer->set_axis(deformer->axis);
    }

    ImGui::End();
}

int main() {

    PandaFramework* framework = new PandaFramework();
    framework->open_framework();
    
    WindowFramework* window = framework->open_window();

    NodePath np = window->load_model(framework->get_models(), "teapot.egg");
    np.reparent_to(window->get_render());

    DirectionalLight *d_light = new DirectionalLight("light");
    NodePath d_light_np = window->get_render().attach_new_node(d_light);
    d_light_np.set_p(-25);
    window->get_render().set_light(d_light_np);

    deformer = new TYPE_DEFORMER(np, Axis::Y);
    deformer->deform_all();
    deformer->set_other(LPoint3f(-5, 5, 0));

    AsyncTaskManager* taskMgr = AsyncTaskManager::get_global_ptr();
    AsyncTask* deform_task = new GenericAsyncTask("deform_task", do_task, deformer);
    taskMgr->add(deform_task);
    window->enable_keyboard();
    window->setup_trackball();

    setup_p3d_imgui(window, render_frame);

    framework->main_loop();
    return 0;
}
