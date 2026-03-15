#include "jpeg_utils.hpp"
#include "jpeg_transformers.hpp"
#include "sample_image_gen.hpp"

#include <any>
#include <chrono>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

// ---------------------------------------------------------------------------
// Result row for the table
// ---------------------------------------------------------------------------

struct ResultRow {
    int quality;
    size_t total_mrs;
    double avg_success_rate;
    size_t perfect_count; // MRs with 100% success
};

// ---------------------------------------------------------------------------
// Run MR search for a given backend + quality
// ---------------------------------------------------------------------------

ResultRow run_quality_level(
    const std::string& backend,
    int quality,
    const std::vector<Image>& sample_images,
    const std::vector<Image>& validation_images,
    size_t input_chain_len,
    size_t output_chain_len,
    double psnr_threshold
) {
    // Build the roundtrip function bound to the given quality
    std::function<Image(Image)> roundtrip;
    if (backend == "stb") {
        roundtrip = [quality](Image img) -> Image {
            return stb_jpeg_roundtrip(img, quality);
        };
    } else {
        roundtrip = [quality](Image img) -> Image {
            return turbo_jpeg_roundtrip(img, quality);
        };
    }

    // Comparer: PSNR >= threshold
    auto psnr_comparer = [psnr_threshold](Image a, Image b) -> bool {
        return compute_psnr(a, b) >= psnr_threshold;
    };

    Aya::MREngine<Image, Image> engine(roundtrip, psnr_comparer);

    auto transformers = make_image_transformers();
    engine.AddInputTransformers(0, transformers);
    engine.AddOutputTransformers(transformers);

    // Sample inputs: all provided sample images for discovery
    std::vector<std::any> sample_pool;
    for (const auto& img : sample_images) {
        sample_pool.push_back(std::any(img));
    }
    std::vector<std::vector<std::any>> sample_inputs = {sample_pool};

    // Verify inputs: the validation images
    std::vector<std::any> verify_pool;
    for (const auto& img : validation_images) {
        verify_pool.push_back(std::any(img));
    }
    std::vector<std::vector<std::any>> verify_inputs = {verify_pool};

    auto mrs = engine.Search(
        sample_inputs, verify_inputs,
        input_chain_len, output_chain_len,
        1, 0, 0.0f, false // Using 0 - 0 as compared indices fails the test -- all MRs are the same.
    );

    // Compute stats
    ResultRow row;
    row.quality = quality;
    row.total_mrs = mrs.size();
    row.avg_success_rate = 0.0;
    row.perfect_count = 0;
    for (const auto& mr : mrs) {
        row.avg_success_rate += mr.LastSuccessRate;
        if (mr.LastSuccessRate >= 1.0f) row.perfect_count++;
    }
    if (!mrs.empty()) row.avg_success_rate /= mrs.size();

    return row;
}

// ---------------------------------------------------------------------------
// Print table
// ---------------------------------------------------------------------------

void print_table(const std::string& backend, const std::vector<ResultRow>& rows) {
    std::cout << "\n" << std::string(65, '=') << "\n";
    std::cout << "Backend: " << backend << "\n";
    std::cout << std::string(65, '-') << "\n";
    std::cout << std::setw(10) << "Quality"
              << std::setw(12) << "Total MRs"
              << std::setw(18) << "Avg Success %"
              << std::setw(15) << "Perfect MRs"
              << "\n";
    std::cout << std::string(65, '-') << "\n";

    for (const auto& r : rows) {
        std::cout << std::setw(10) << r.quality
                  << std::setw(12) << r.total_mrs
                  << std::setw(17) << std::fixed << std::setprecision(1)
                  << (r.avg_success_rate * 100.0) << "%"
                  << std::setw(15) << r.perfect_count
                  << "\n";
    }
    std::cout << std::string(65, '=') << "\n";
}

// ---------------------------------------------------------------------------
// Write CSV
// ---------------------------------------------------------------------------

void write_csv(const std::string& path, const std::string& backend,
               const std::vector<ResultRow>& rows, double psnr_threshold) {
    std::ofstream f(path);
    f << "backend,psnr_threshold,quality,total_mrs,avg_success_rate,perfect_count\n";
    for (const auto& r : rows) {
        f << backend << "," << std::fixed << std::setprecision(1) << psnr_threshold
          << "," << r.quality << "," << r.total_mrs << ","
          << std::setprecision(4) << r.avg_success_rate << ","
          << r.perfect_count << "\n";
    }
    std::cout << "CSV written to: " << path << "\n";
}

// ---------------------------------------------------------------------------
// Main
// ---------------------------------------------------------------------------

int main(int argc, char* argv[]) {
    std::cout << "Aya JPEG Compression MR Test\n";
    std::cout << std::string(65, '=') << "\n";

    // Configuration
    size_t input_chain_len = 2;
    size_t output_chain_len = 2;
    std::string image_dir = "test_images";
    bool run_stb = true;
    bool run_turbo = true;
    double psnr_threshold = 25.0;

    // Parse simple args
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--stb-only") { run_turbo = false; }
        else if (arg == "--turbo-only") { run_stb = false; }
        else if (arg == "--image-dir" && i + 1 < argc) { image_dir = argv[++i]; }
        else if (arg == "--input-chain" && i + 1 < argc) { input_chain_len = std::stoul(argv[++i]); }
        else if (arg == "--output-chain" && i + 1 < argc) { output_chain_len = std::stoul(argv[++i]); }
        else if (arg == "--psnr" && i + 1 < argc) { psnr_threshold = std::stod(argv[++i]); }
        else if (arg == "--help") {
            std::cout << "Usage: jpeg_mr_test [OPTIONS]\n"
                      << "  --stb-only         Only run stb backend\n"
                      << "  --turbo-only       Only run TurboJPEG backend\n"
                      << "  --image-dir DIR    Path to test images (default: test_images)\n"
                      << "  --input-chain N    Input chain length (default: 2)\n"
                      << "  --output-chain N   Output chain length (default: 2)\n"
                      << "  --psnr THRESHOLD   PSNR threshold in dB (default: 25.0)\n";
            return 0;
        }
    }

    // Generate synthetic sample images
    std::cout << "Generating checkerboard (512x512)...\n";
    Image checkerboard = generate_checkerboard(512, 32);
    std::cout << "Generating gradient (512x512)...\n";
    Image gradient = generate_gradient(512, 512);
    std::cout << "Generating noise block (512x512)...\n";
    Image noise_block = generate_noise_block(512, 512);
    std::cout << "Generating diagonal gradient (512x512)...\n";
    Image diagonal_grad = generate_diagonal_gradient(512, 512);

    std::vector<Image> sample_images = {checkerboard, gradient, noise_block, diagonal_grad};

    // Scan for validation images (all PNGs in image_dir)
    std::vector<Image> validation_images;

    if (std::filesystem::exists(image_dir) && std::filesystem::is_directory(image_dir)) {
        for (const auto& entry : std::filesystem::directory_iterator(image_dir)) {
            if (entry.is_regular_file() && entry.path().extension() == ".png") {
                std::cout << "Loading " << entry.path().string() << "...\n";
                validation_images.push_back(load_image(entry.path().string()));
            }
        }
    }

    if (validation_images.empty()) {
        std::cout << "No validation images found in " << image_dir << "/. Using synthetic images for verification.\n";
        validation_images = sample_images;
    }

    std::cout << "Loaded " << validation_images.size() << " validation image(s), "
              << sample_images.size() << " sample image(s).\n";
    std::cout << "Chain lengths: input=" << input_chain_len
              << ", output=" << output_chain_len << "\n";
    std::cout << "PSNR threshold: " << psnr_threshold << " dB\n\n";

    // Ensure results directory exists (with PSNR subdirectory)
    std::string results_dir = "results/psnr_" + std::to_string(static_cast<int>(psnr_threshold));
    std::filesystem::create_directories(results_dir);

    std::vector<int> qualities = {10, 20, 30, 40, 50, 60, 70, 80, 90};

    // stb backend
    if (run_stb) {
        std::cout << "Running stb backend...\n";
        std::vector<ResultRow> stb_rows;
        for (int q : qualities) {
            auto start = std::chrono::steady_clock::now();
            std::cout << "  Quality " << q << "... " << std::flush;
            auto row = run_quality_level("stb", q, sample_images, validation_images,
                                          input_chain_len, output_chain_len, psnr_threshold);
            auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::steady_clock::now() - start).count();
            std::cout << row.total_mrs << " MRs (" << elapsed << " ms)\n";
            stb_rows.push_back(row);
        }
        print_table("stb", stb_rows);
        write_csv(results_dir + "/stb_results.csv", "stb", stb_rows, psnr_threshold);
    }

    // TurboJPEG backend
    if (run_turbo) {
        std::cout << "\nRunning TurboJPEG backend...\n";
        std::vector<ResultRow> turbo_rows;
        for (int q : qualities) {
            auto start = std::chrono::steady_clock::now();
            std::cout << "  Quality " << q << "... " << std::flush;
            auto row = run_quality_level("turbo", q, sample_images, validation_images,
                                          input_chain_len, output_chain_len, psnr_threshold);
            auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::steady_clock::now() - start).count();
            std::cout << row.total_mrs << " MRs (" << elapsed << " ms)\n";
            turbo_rows.push_back(row);
        }
        print_table("TurboJPEG", turbo_rows);
        write_csv(results_dir + "/turbo_results.csv", "turbo", turbo_rows, psnr_threshold);
    }

    std::cout << "\nDone.\n";
    return 0;
}
