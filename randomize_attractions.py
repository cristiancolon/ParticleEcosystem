import random

# Generate randomized values directly
def generate_random_matrix(num_values, low=1, high=5):
    values = []
    for _ in range(num_values):
        rand_val = random.uniform(low, high)
        rounded_val = round(rand_val * 2) / 2  # clamp to nearest .5
        if random.random() < 0.5:  # 50% chance to negate
            rounded_val *= -1
        values.append(rounded_val)
    return values

def write_attraction_matrix_to_file(filename="attraction_matrix.txt"):
    # Color species in order (matching the enum in Color.h)
    species = ['RED', 'GREEN', 'BLUE', 'YELLOW', 'CYAN', 'MAGENTA', 'PURPLE', 'ORANGE']
    
    # Generate 64 random values (8x8 matrix)
    values = generate_random_matrix(64)
    
    # Write to file in a simple format: from_species to_species attraction_value
    with open(filename, 'w') as f:
        f.write("# Attraction matrix data\n")
        f.write("# Format: from_species to_species attraction_value\n")
        f.write(f"# Species order: {' '.join(species)}\n")
        f.write("\n")
        
        value_index = 0
        for i, species1 in enumerate(species):
            for j, species2 in enumerate(species):
                value = values[value_index]
                f.write(f"{i} {j} {value}\n")
                value_index += 1
    
    print(f"Attraction matrix written to {filename}")
    return filename

def generate_attraction_matrix_string():
    # Color species in order
    species = ['RED', 'GREEN', 'BLUE', 'YELLOW', 'CYAN', 'MAGENTA', 'PURPLE', 'ORANGE']
    
    # Generate 64 random values (8x8 matrix)
    values = generate_random_matrix(64)
    
    # Build the matrix string
    matrix_string = "const AttractionMatrix attractionMatrix = {\n"
    
    value_index = 0
    for i, species1 in enumerate(species):
        for j, species2 in enumerate(species):
            value = values[value_index]
            matrix_string += f"        {{ColorSpecies::{species1}, ColorSpecies::{species2}, {value}f}},\n"
            value_index += 1
        
        # Add blank line between species groups for readability
        if i < len(species) - 1:
            matrix_string += "\n"
    
    matrix_string += "    };"
    
    return matrix_string

if __name__ == "__main__":
    # Write to file instead of printing C++ code
    write_attraction_matrix_to_file()
    print("Run the C++ program to use the generated attraction matrix.")