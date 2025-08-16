#pragma once

#include <vector>
#include <tuple>
#include <unordered_map>
#include <functional>

namespace Color {

    struct Color {
        float r;
        float g;
        float b;
        bool operator==(const Color& other) const {
            return r == other.r && g == other.g && b == other.b;
        }
        bool operator!=(const Color& other) const {
            return !(*this == other);
        }
    };

    struct ColorHash {
        std::size_t operator()(const Color& color) const noexcept {
            std::size_t h1 = std::hash<float>{}(color.r);
            std::size_t h2 = std::hash<float>{}(color.g);
            std::size_t h3 = std::hash<float>{}(color.b);
            return h1 ^ (h2 << 1) ^ (h3 << 2);
        }
    };
    struct ColorEq {
        bool operator()(const Color& a, const Color& b) const noexcept {
            return a.r == b.r && a.g == b.g && a.b == b.b;
        }
    };

    using ColorMap  = std::unordered_map<Color, float, ColorHash, ColorEq>;
    using ColorMM   = std::unordered_map<Color, ColorMap, ColorHash, ColorEq>;

    enum ColorSpecies {
        RED,
        GREEN,
        BLUE,
        YELLOW,
        CYAN,
        MAGENTA,
        PURPLE,
        ORANGE,
    };

    constexpr int NUM_SPECIES = 8;

    const std::unordered_map<ColorSpecies, Color> colorMap = {
		{ColorSpecies::RED, Color{ 1.0f, 0.0f, 0.0f }},
		{ColorSpecies::GREEN, Color{ 0.0f, 1.0f, 0.0f }},
		{ColorSpecies::BLUE, Color{ 0.0f, 0.0f, 1.0f }},
		{ColorSpecies::YELLOW, Color{ 1.0f, 1.0f, 0.0f }},
		{ColorSpecies::CYAN, Color{ 0.0f, 1.0f, 1.0f }},
		{ColorSpecies::MAGENTA, Color{ 1.0f, 0.0f, 1.0f }},
		{ColorSpecies::PURPLE, Color{ 0.5f, 0.0f, 1.0f }},
		{ColorSpecies::ORANGE, Color{ 1.0f, 0.5f, 0.0f }},
	};

    inline ColorSpecies colorToSpecies(const Color& color) {
        // Map colors to species indices (0-5 for the 6 ColorSpecies enum values)
        if (color.r == 1.0f && color.g == 0.0f && color.b == 0.0f) return ColorSpecies::RED;
        if (color.r == 0.0f && color.g == 1.0f && color.b == 0.0f) return ColorSpecies::GREEN;
        if (color.r == 0.0f && color.g == 0.0f && color.b == 1.0f) return ColorSpecies::BLUE;
        if (color.r == 1.0f && color.g == 1.0f && color.b == 0.0f) return ColorSpecies::YELLOW;
        if (color.r == 0.0f && color.g == 1.0f && color.b == 1.0f) return ColorSpecies::CYAN;
        if (color.r == 1.0f && color.g == 0.0f && color.b == 1.0f) return ColorSpecies::MAGENTA;
        if (color.r == 0.5f && color.g == 0.0f && color.b == 1.0f) return ColorSpecies::PURPLE;
        if (color.r == 1.0f && color.g == 0.5f && color.b == 0.0f) return ColorSpecies::ORANGE;
        return ColorSpecies::RED; // default to RED if no match
    };

    using AttractionRow = std::tuple<ColorSpecies, ColorSpecies, float>;
    using AttractionMatrix = std::vector<AttractionRow>;

    const AttractionMatrix attractionMatrix = {
        {ColorSpecies::RED, ColorSpecies::RED, 3.0f},
        {ColorSpecies::RED, ColorSpecies::GREEN, -1.5f},
        {ColorSpecies::RED, ColorSpecies::BLUE, 4.5f},
        {ColorSpecies::RED, ColorSpecies::YELLOW, 3.0f},
        {ColorSpecies::RED, ColorSpecies::CYAN, -2.5f},
        {ColorSpecies::RED, ColorSpecies::MAGENTA, -4.5f},
        {ColorSpecies::RED, ColorSpecies::PURPLE, 4.5f},
        {ColorSpecies::RED, ColorSpecies::ORANGE, 4.0f},

        {ColorSpecies::GREEN, ColorSpecies::RED, 3.5f},
        {ColorSpecies::GREEN, ColorSpecies::GREEN, 3.0f},
        {ColorSpecies::GREEN, ColorSpecies::BLUE, 4.0f},
        {ColorSpecies::GREEN, ColorSpecies::YELLOW, -2.5f},
        {ColorSpecies::GREEN, ColorSpecies::CYAN, -1.5f},
        {ColorSpecies::GREEN, ColorSpecies::MAGENTA, 1.5f},
        {ColorSpecies::GREEN, ColorSpecies::PURPLE, 3.5f},
        {ColorSpecies::GREEN, ColorSpecies::ORANGE, 3.0f},

        {ColorSpecies::BLUE, ColorSpecies::RED, -2.0f},
        {ColorSpecies::BLUE, ColorSpecies::GREEN, 1.5f},
        {ColorSpecies::BLUE, ColorSpecies::BLUE, 4.0f},
        {ColorSpecies::BLUE, ColorSpecies::YELLOW, -2.0f},
        {ColorSpecies::BLUE, ColorSpecies::CYAN, -3.5f},
        {ColorSpecies::BLUE, ColorSpecies::MAGENTA, 2.5f},
        {ColorSpecies::BLUE, ColorSpecies::PURPLE, -4.0f},
        {ColorSpecies::BLUE, ColorSpecies::ORANGE, -4.5f},

        {ColorSpecies::YELLOW, ColorSpecies::RED, -3.5f},
        {ColorSpecies::YELLOW, ColorSpecies::GREEN, 3.5f},
        {ColorSpecies::YELLOW, ColorSpecies::BLUE, 5.0f},
        {ColorSpecies::YELLOW, ColorSpecies::YELLOW, 3.0f},
        {ColorSpecies::YELLOW, ColorSpecies::CYAN, 4.0f},
        {ColorSpecies::YELLOW, ColorSpecies::MAGENTA, -2.5f},
        {ColorSpecies::YELLOW, ColorSpecies::PURPLE, 4.0f},
        {ColorSpecies::YELLOW, ColorSpecies::ORANGE, 4.0f},

        {ColorSpecies::CYAN, ColorSpecies::RED, 4.5f},
        {ColorSpecies::CYAN, ColorSpecies::GREEN, -3.5f},
        {ColorSpecies::CYAN, ColorSpecies::BLUE, -3.0f},
        {ColorSpecies::CYAN, ColorSpecies::YELLOW, -2.0f},
        {ColorSpecies::CYAN, ColorSpecies::CYAN, -1.5f},
        {ColorSpecies::CYAN, ColorSpecies::MAGENTA, -2.5f},
        {ColorSpecies::CYAN, ColorSpecies::PURPLE, 4.5f},
        {ColorSpecies::CYAN, ColorSpecies::ORANGE, -4.0f},

        {ColorSpecies::MAGENTA, ColorSpecies::RED, 1.5f},
        {ColorSpecies::MAGENTA, ColorSpecies::GREEN, 1.0f},
        {ColorSpecies::MAGENTA, ColorSpecies::BLUE, 3.5f},
        {ColorSpecies::MAGENTA, ColorSpecies::YELLOW, -4.5f},
        {ColorSpecies::MAGENTA, ColorSpecies::CYAN, 4.5f},
        {ColorSpecies::MAGENTA, ColorSpecies::MAGENTA, -4.0f},
        {ColorSpecies::MAGENTA, ColorSpecies::PURPLE, -1.5f},
        {ColorSpecies::MAGENTA, ColorSpecies::ORANGE, -4.5f},

        {ColorSpecies::PURPLE, ColorSpecies::RED, -2.0f},
        {ColorSpecies::PURPLE, ColorSpecies::GREEN, 4.0f},
        {ColorSpecies::PURPLE, ColorSpecies::BLUE, -5.0f},
        {ColorSpecies::PURPLE, ColorSpecies::YELLOW, -2.5f},
        {ColorSpecies::PURPLE, ColorSpecies::CYAN, 5.0f},
        {ColorSpecies::PURPLE, ColorSpecies::MAGENTA, 3.0f},
        {ColorSpecies::PURPLE, ColorSpecies::PURPLE, -2.0f},
        {ColorSpecies::PURPLE, ColorSpecies::ORANGE, -1.0f},

        {ColorSpecies::ORANGE, ColorSpecies::RED, 3.5f},
        {ColorSpecies::ORANGE, ColorSpecies::GREEN, -3.0f},
        {ColorSpecies::ORANGE, ColorSpecies::BLUE, 4.0f},
        {ColorSpecies::ORANGE, ColorSpecies::YELLOW, 3.0f},
        {ColorSpecies::ORANGE, ColorSpecies::CYAN, -3.0f},
        {ColorSpecies::ORANGE, ColorSpecies::MAGENTA, 4.0f},
        {ColorSpecies::ORANGE, ColorSpecies::PURPLE, 3.0f},
        {ColorSpecies::ORANGE, ColorSpecies::ORANGE, 3.0f},
    };



}