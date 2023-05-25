//
// Created by juperez on 5/23/23.
//
#include <iostream>
#include <filesystem>
#include <tbb/task_scheduler_observer.h>

#include "utils/parser.h"

using namespace lumina;

static int numThreads = -1;
static bool useGui = true;

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
            if (path.extension() == "xml") {
                sceneFileName = argv[i];
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
