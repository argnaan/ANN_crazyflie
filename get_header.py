import torch
import torch.nn as nn

header_path = "ANN_crazyflie/src/pesi_modello.h"
model_path = "best_agent_noise01.pt"

def tensor_to_c_matrix(tensor, name):
    """Converte un tensore 1D o 2D in una dichiarazione di matrice C"""
    array = tensor.cpu().numpy()
    shape = array.shape

    c_code = f"// Shape: {shape}\n"
    
    if len(shape) == 1:
        # Vettore 1D
        c_code += f"const float {name}[{shape[0]}] = {{\n    "
        c_code += ", ".join(f"{x:.6f}" for x in array)
        c_code += "\n};\n"
    
    elif len(shape) == 2:
        # Matrice 2D
        c_code += f"const float {name}[{shape[0]}][{shape[1]}] = {{\n"
        for row in array:
            c_code += "    { " + ", ".join(f"{x:.6f}" for x in row) + " },\n"
        c_code += "};\n"
    
    else:
        raise NotImplementedError(f"Shape {shape} non supportata.")

    return c_code

def shape_to_define(name, shape):
    defines = []
    for i, dim in enumerate(shape):
        defines.append(f"#define {name.upper()}_DIM{i} {dim}")
    return "\n".join(defines)

def save_weights_to_header(model_path, header_path):
    model = torch.load(model_path, map_location="cpu")
    with open(header_path, "w") as f:
        f.write("// Auto-generated header with model weights and dimensions\n\n")
        for key, tensor in model['value'].items():
            sanitized_name = key.replace('.', '_')
            # Macro per dimensioni
            f.write(shape_to_define(sanitized_name, tensor.shape) + "\n\n")
            # Matrice o vettore
            f.write(tensor_to_c_matrix(tensor, sanitized_name) + "\n")


save_weights_to_header(model_path, header_path)
print(f"Scrittura di {header_path} completata")
