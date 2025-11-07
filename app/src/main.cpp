#include "SDL3/SDL_video.h"
#include <format>
#include <tuple>
#include <variant>

#define SDL_MAIN_USE_CALLBACKS
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include "imgui.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_sdlrenderer3.h"

#include <rdpp_client/VideoDecoder.hpp>
#include <rdpp_server/VideoStreamer.hpp>
#include <rdpp_common/Logging.hpp>
#include <rdpp_server/ScreenRecorder.hpp>

using namespace rdpp;
using rdpp::common::log::printrel;
using rdpp::common::log::printdbg;

struct AppState {
    SDL_Window *window;
    SDL_Renderer *renderer;

    SDL_Texture *texture = nullptr;
    std::variant<client::VideoDecoder, server::VideoStreamer, std::monostate> video = std::monostate{};
    std::optional<server::ScreenRecorder> recorder;
};

SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[]) {
    printdbg<int>("The answer is {}", {42});

    AppState *state = new AppState;

    /* Create the window */
    if (!SDL_CreateWindowAndRenderer("Hello World", 800, 600, SDL_WINDOW_MAXIMIZED | SDL_WINDOW_RESIZABLE, &state->window, &state->renderer)) {
        SDL_Log("Couldn't create window and renderer: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    state->texture = SDL_CreateTexture(state->renderer, SDL_PIXELFORMAT_IYUV, SDL_TEXTUREACCESS_STREAMING, 1920, 1080);
    if (!state->texture) {
        SDL_Log("Couldn't create texture: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    ImGui::StyleColorsDark();
    ImGui_ImplSDL3_InitForSDLRenderer(state->window, state->renderer);
    ImGui_ImplSDLRenderer3_Init(state->renderer);

    std::cout << "This thread id: " << std::this_thread::get_id() << std::endl;

    *appstate = state;

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event) {
    AppState &state = *static_cast<AppState*>(appstate);

    if (event->type == SDL_EVENT_QUIT)
        return SDL_APP_SUCCESS;

    ImGui_ImplSDL3_ProcessEvent(event);
    if (ImGui::GetIO().WantCaptureMouse) return SDL_APP_CONTINUE;
    if (ImGui::GetIO().WantCaptureKeyboard) return SDL_APP_CONTINUE;

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void *appstate) {
    AppState &state = *static_cast<AppState*>(appstate);
    SDL_Renderer *renderer = state.renderer;

    ImGui_ImplSDL3_NewFrame();
    ImGui_ImplSDLRenderer3_NewFrame();
    ImGui::NewFrame();

    auto curr_state = state.video.index();

    if (curr_state == 0) {
        auto& decoder = std::get<0>(state.video);
        auto frame = decoder.getLatestFrame();

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        if (frame) {
            if (SDL_GetTicks() % 1000 == 0)
                printdbg("Frame: {}x{}", std::make_tuple(frame->width, frame->height));

            SDL_UpdateYUVTexture(state.texture,
                                NULL,
                                frame->data[0], frame->linesize[0],
                                frame->data[1], frame->linesize[1],
                                frame->data[2], frame->linesize[2]);
            SDL_RenderTexture(renderer, state.texture, NULL, NULL);
        }
    } else if (curr_state == 2) {
        ImGui::Begin("Choose application type");

        if (ImGui::Button("Server")) {
            const auto& monitors = SL::Screen_Capture::GetMonitors();
            state.recorder.emplace(monitors[0]);
            state.recorder->start();

            state.video.emplace<server::VideoStreamer>("C:\\Users\\Atharv\\outputx264.mkv", "udp://localhost:9999");
            if (!std::get<server::VideoStreamer>(state.video).start())
                return SDL_APP_FAILURE;
        }

        if (ImGui::Button("Client")) {
            state.video.emplace<client::VideoDecoder>("udp://localhost:9999");
            std::get<client::VideoDecoder>(state.video).start();
        }

        ImGui::End();
    }

    ImGui::Render();
    ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), renderer);

    SDL_RenderPresent(renderer);

    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *appstate, SDL_AppResult result) {
    AppState *state = static_cast<AppState *>(appstate);
    ImGui_ImplSDL3_Shutdown();
    ImGui_ImplSDLRenderer3_Shutdown();
    ImGui::DestroyContext();
    SDL_DestroyRenderer(state->renderer);
    SDL_DestroyWindow(state->window);
    delete state;
}

