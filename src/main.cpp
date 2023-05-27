//
// Created by juperez on 5/23/23.
//
#include <iostream>
#include <filesystem>
#include <tbb/task_scheduler_observer.h>
#include <tbb/parallel_for.h>

#include "utils/parser.h"
#include "scene/scene.h"
#include "utils/timer.h"
#include "tbb/blocked_range.h"
#include "image/gui.h"

using namespace lumina;

static int numThreads = -1;
static bool useGui = true;

static void renderBlock(const Scene* scene, Sampler* sampler, ImageBlock& block) {
    const Camera* camera = scene->getCamera();
    const Integrator* integrator = scene->getIntegrator();

    Point2i offset = block.getOffset();
    Vector2i size = block.getSize();

    block.clear();

    for (int y = 0; y < size.y(); y++) {
        for (int x = 0; x < size.x(); x++) {
            for (uint32_t i = 0; i < sampler->getSampleCount(); i++) {
                Point2f pixelSample = Point2f(
                        (float) (x + offset.x()),
                        (float) (y + offset.y())
                        ) + sampler->next2D();
                Point2f apertureSample = sampler->next2D();

                Ray3f ray;
                Color3f value = camera->sampleRay(ray, pixelSample, apertureSample);

                value *= integrator->Li(scene, sampler, ray);

                block.put(pixelSample, value);
            }
        }
    }

}

static void render(Scene* scene, const std::string& filename) {
    const Camera* camera = scene->getCamera();
    Vector2i outputSize = camera->getOutputSize();
    scene->getIntegrator()->preprocess(scene);

    BlockGenerator blockGenerator(outputSize, LUMINA_BLOCK_SIZE);

    ImageBlock result(outputSize, camera->getReconstructionFilter());
    result.clear();

    LuminaScreen* screen = nullptr;
    if (useGui) {
        nanogui::init();
        screen = new LuminaScreen(result);
    }

    std::thread render_thread([&] {
        tbb::task_arena init(numThreads);

        std::cout << "Rendering...";
        std::cout.flush();

        Timer timer;

        tbb::blocked_range<int> range(0, blockGenerator.getBlockCount());

        auto map = [&](const tbb::blocked_range<int>& range) {
            ImageBlock block(Vector2i(LUMINA_BLOCK_SIZE), camera->getReconstructionFilter());

            std::unique_ptr<Sampler> sampler(scene->getSampler()->clone());

            for (int i = range.begin(); i < range.end(); i++) {
                blockGenerator.next(block);

                sampler->prepare(block);

                renderBlock(scene, sampler.get(), block);

                result.put(block);
            }
        };

        tbb::parallel_for(range, map);

        std::cout << " done. (took " << timer.elapsedString() << ") \n";
    });

    if (useGui)
        nanogui::mainloop(50.0f);

    render_thread.join();

    if (useGui) {
        delete screen;
        nanogui::shutdown();
    }

    std::unique_ptr<Bitmap> bitmap(result.toBitmap());
    std::string outputName = filename;
    size_t lastdot = outputName.find_last_of(".");
    if (lastdot != std::string::npos)
        outputName.erase(lastdot, std::string::npos);

    bitmap->savePNG(outputName);
    bitmap->saveEXR(outputName);
}

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cout << "Syntax: " << argv[0] << " <scene.xml> [--no-gui] [--threads N]\n";
    }

    std::string sceneFileName = "";

    for (int i = 0; i < argc; i++) {
        std::string token(argv[i]);

        if (token == "--threads") {
            if (i+1 >= argc) {
                std::cerr << "--threads expected a positive integer after the argument \n";
                return -1;
            }
            numThreads = atoi(argv[i+1]);
            i++;

            if (numThreads < 0) {
                std::cerr << "--threads expected a positive integer for the number of threads \n";
                return -1;
            }

            continue;
        } else if (token == "--no-gui") {
            useGui = false;
            continue;
        }

        std::filesystem::path path(argv[i]);

        try {
            if (path.extension() == ".xml") {
                sceneFileName = argv[i];

                getFileResolver()->prepend(path.parent_path());
            } else {
                std::cerr << "Error: unknown file format " << argv[i]
                << ", expected file format .xml \n";
            }
        } catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << "\n";
            return -1;
        }
    }

    if (sceneFileName != "") {
        if (numThreads < 0) {
            numThreads = tbb::info::default_concurrency();
        }

        try {
            std::unique_ptr<LuminaObject> root(loadXMLFile(sceneFileName));

            if (root->getClassType() == LuminaObject::EScene)
                render(static_cast<Scene*>(root.get()), sceneFileName);
        } catch (const std::exception& e) {
            std::cerr << e.what() << "\n";
            return -1;
        }
    }
    return 0;
}
