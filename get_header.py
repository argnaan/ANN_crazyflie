import torch
import torch.nn as nn

header_path = "src/pesi_modello.h"
model_path = "best_agent_noise01.pt"


def tensor_to_c_matrix(tensor, i_layer):
    """Converte un tensore 1D o 2D in una dichiarazione di matrice C"""
    array = tensor.cpu().numpy()
    shape = array.shape

    c_code = f"// Shape: {shape}\n"
    
    if len(shape) == 1:
        # Vettore 1D
        c_code += f"#define NET_{i_layer}_BIAS_DIM {shape[0]}\n"
        c_code += f"const float net_{i_layer}_bias[{shape[0]}] = {{\n    "
        c_code += ", ".join(f"{x:.6f}" for x in array)
        c_code += "\n};\n"
    
    elif len(shape) == 2:
        # Matrice 2D
        c_code += f"#define NET_{i_layer}_WEIGHT_DIM0 {shape[0]}\n"
        c_code += f"#define NET_{i_layer}_WEIGHT_DIM1 {shape[1]}\n"
        c_code += f"const float net_{i_layer}_weight[{shape[0]}][{shape[1]}] = {{\n"
        for row in array:
            c_code += "    { " + ", ".join(f"{x:.6f}" for x in row) + " },\n"
        c_code += "};\n"
        i_layer += 1
    
    else:
        raise NotImplementedError(f"Shape {shape} non supportata.")

    return c_code

def save_weights_to_header(model_path, header_path):
    model = torch.load(model_path, map_location="cpu")
    with open(header_path, "w") as f:
        f.write("// Auto-generated header with model weights and dimensions\n\n")
        i_layer = 0
        add_i = False
        first = True
        for key, tensor in model['value'].items():
            if first:
                first = False
                continue
            f.write(tensor_to_c_matrix(tensor, i_layer) + "\n")
            if add_i:
                i_layer += 1
                add_i = False
            else:
                add_i = True

        f.write(f"#define N_LAYER {i_layer-1}\n")
        f.write("#define TEST_ANN 1\n")


save_weights_to_header(model_path, header_path)
print(f"Scrittura di {header_path} completata")
