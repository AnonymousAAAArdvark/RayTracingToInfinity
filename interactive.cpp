//
// Created by Andrew Yang on 3/27/21.
//

#include <iostream>
#include <thread>
#include <vector>

#include <SFML/Graphics.hpp>

#include "raytracer.hpp"
#include "timer.hpp"
#include "parallel/pixels.hpp"
#include "parallel/task.hpp"
#include "parallel/params.hpp"
#include "hittable/2dhittables.hpp"

float params::ASPECT_RATIO = 1.0f;
unsigned params::WIDTH = 100;
unsigned params::HEIGHT = int(params::WIDTH / params::ASPECT_RATIO);

unsigned params::N = 10;//16;
unsigned params::N_samples = 200;
unsigned params::MAX_DEPTH = 16;

unsigned params::W_CNT = (params::WIDTH + params::N - 1) / params::N;
unsigned params::H_CNT = (params::HEIGHT + params::N - 1) / params::N;

int main() {
    hittable_list world;

    point3 lookfrom;
    point3 lookat;
    auto vfov = 40.0f;
    auto aperture = .0f;
    color background(0, 0, 0);

    switch(0) {
        case 1:
            world = random_scene();
            background = color(.7f, .8f, 1.0f);
            lookfrom = point3(13, 2, 3);
            lookat = point3(0, 0, 0);
            vfov = 20.0f;
            aperture = .1f;
            break;
        case 2:
            world = two_spheres();
            background = color(.7f, .8f, 1.0f);
            lookfrom = point3(13, 2, 3);
            lookat = point3(0, 0, 0);
            vfov = 20.0f;
            break;
        case 3:
            world = two_perlin_spheres();
            background = color(.7f, .8f, 1.0f);
            lookfrom = point3(13, 2, 3);
            lookat = point3(0, 0, 0);
            vfov = 20.0f;
            break;
        case 4:
            world = earth();
            background = color(.7f, .8f, 1.0f);
            lookfrom = point3(13, 2, 3);
            lookat = point3(0, 0, 0);
            vfov = 20.0f;
            break;
        case 5:
            world = simple_light();
            background = color(0.0f, 0.0f, 0.0f);
            lookfrom = point3(26, 3, 6);
            lookat = point3(0, 2, 0);
            vfov = 20.0f;
            break;
        case 6:
            world = cornell_box();
            background = color(0, 0, 0);
            lookfrom = point3(278, 278, -800);
            lookat = point3(278, 278, 0);
            vfov = 40.0f;
            break;
        case 7:
            world = cornell_smoke();
            lookfrom = point3(278, 278, -800);
            lookat = point3(278, 278, 0);
            vfov = 40.0f;
            break;
        case 8:
            world = final_scene();
            background = color(0,0,0);
            lookfrom = point3(478, 278, -600);
            lookat = point3(278, 278, 0);
            vfov = 40.0f;
            break;
        default:
        case 9:
            world = single_cylinder();
            background = color(.7f, .8f, 1.0f);
            lookfrom = point3(13, 2, 3);
            lookat = point3(0, 0, 0);
            vfov = 20.0f;
            aperture = .1f;
            break;
    }

    // Render window

    int window_w = 1024;
    if(window_w > params::WIDTH) window_w = (int)params::WIDTH;
    sf::RenderWindow window(sf::VideoMode(window_w, (window_w/params::ASPECT_RATIO)),
                            "Ray Tracing", sf::Style::Titlebar | sf::Style::Close);

    sf::Texture tex;
    sf::Sprite sprite;

    if(!tex.create(params::WIDTH, params::HEIGHT)) {
        std::cerr << "Couldn't create texture!" << std::endl;
        return 1;
    }

    tex.setSmooth(false);

    sprite.setTexture(tex);

    pixels pix = pixels(params::WIDTH, params::HEIGHT);
    std::vector<float> data(params::WIDTH * params::HEIGHT * 5);

    // Camera

    vec3 vup(0, 1, 0);
    auto dist_to_focus = 10.0f;
    camera cam(lookfrom, lookat, vup, vfov, params::ASPECT_RATIO, aperture, dist_to_focus, .0f, 1.0f);

    const unsigned int n_threads = std::thread::hardware_concurrency();
    std::cout << "Detected " << n_threads << " concurrent threads." << std::endl;
    std::vector<std::thread> threads(n_threads);

    Timer timer;

    for(auto& t : threads) t = std::thread(Task{world, cam, background, &data[0]});

    bool finished_rendering = false;

    while(window.isOpen()) {
        sf::Event event{};
        while(window.pollEvent(event)) {
            if(event.type == sf::Event::Closed) window.close();
        }

        tex.update(&pix.get_pixels(data)[0]);

        window.clear();
        window.draw(sprite);
        window.display();

        if(!finished_rendering && done_count == n_threads) {
            std::cout << "Finished rendering in " << timer.get_millis() << " ms or "
                      << timer.get_seconds() << "s. " << std::endl;
            finished_rendering = true;
        }

        sf::sleep(sf::milliseconds(1000));
    }

    std::cout << "Waiting for all threads to join" << std::endl;
    for(auto& t : threads) t.join();

    tex.copyToImage().saveToFile("output/out.png");
    std::cout << "Saved image to out.png" << std::endl;

    return 0;
}