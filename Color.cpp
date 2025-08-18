#include "Color.h"

namespace Color {
    
    // Define the global attraction matrix
    AttractionMatrix attractionMatrix;

    bool loadAttractionMatrixFromFile(const std::string& filename) {
        std::ifstream file(filename);
        if (!file.is_open()) {
            std::cerr << "Warning: Could not open attraction matrix file '" << filename 
                      << "'. Using default matrix." << std::endl;
            attractionMatrix = getDefaultAttractionMatrix();
            return false;
        }

        attractionMatrix.clear();
        std::string line;
        
        while (std::getline(file, line)) {
            // Skip empty lines and comments
            if (line.empty() || line[0] == '#') {
                continue;
            }

            std::istringstream iss(line);
            int fromSpecies, toSpecies;
            float attractionValue;
            
            if (iss >> fromSpecies >> toSpecies >> attractionValue) {
                // Validate species indices
                if (fromSpecies >= 0 && fromSpecies < NUM_SPECIES && 
                    toSpecies >= 0 && toSpecies < NUM_SPECIES) {
                    
                    attractionMatrix.emplace_back(
                        static_cast<ColorSpecies>(fromSpecies),
                        static_cast<ColorSpecies>(toSpecies),
                        attractionValue
                    );
                } else {
                    std::cerr << "Warning: Invalid species indices in line: " << line << std::endl;
                }
            } else {
                std::cerr << "Warning: Could not parse line: " << line << std::endl;
            }
        }

        file.close();

        // Verify we have the expected number of entries (8x8 = 64)
        if (attractionMatrix.size() != NUM_SPECIES * NUM_SPECIES) {
            std::cerr << "Warning: Expected " << (NUM_SPECIES * NUM_SPECIES) 
                      << " entries, but got " << attractionMatrix.size() 
                      << ". Using default matrix." << std::endl;
            attractionMatrix = getDefaultAttractionMatrix();
            return false;
        }

        std::cout << "Successfully loaded attraction matrix from '" << filename 
                  << "' with " << attractionMatrix.size() << " entries." << std::endl;
        return true;
    }
}
