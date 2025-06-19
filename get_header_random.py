import random

header_path = "src/pesi_modello_rnd.h"

layers_dim = [17, 128, 256, 64, 4]

if __name__ == "__main__":
    header = open(header_path, "w")
    header.write("// Auto-generated header with model weights and dimensions\n\n")
    header.write("#define N_LAYER " + str(len(layers_dim) - 1) + "\n\n")
    header.write("#define TEST_ANN 0\n\n")
    
    for i in range(len(layers_dim) - 1):
        header.write("#define NET_" + str(i) + "_WEIGHT_DIM0 " + str(layers_dim[i+1]) + "\n")
        header.write("#define NET_" + str(i) + "_WEIGHT_DIM1 " + str(layers_dim[i]) + "\n")
        header.write("#define NET_" + str(i) + "_BIAS_DIM " + str(layers_dim[i + 1]) + "\n")
        header.write("const float net_" + str(i) + "_weight[" + str(layers_dim[i+1]) + "][" + str(layers_dim[i]) + "] = {\n")
        for j in range(layers_dim[i + 1]):
            header.write("    {")
            for k in range(layers_dim[i]):
                if k < layers_dim[i] - 1:
                    header.write(f"{random.uniform(-0.1, 0.1)}, ")
            header.write(f"{random.uniform(-0.1, 0.1)}")
            if j < layers_dim[i + 1] - 1:
                header.write("},\n")
            else:
                header.write("}\n};\n\n")
        
        header.write("const float net_" + str(i) + "_bias[" + str(layers_dim[i + 1]) + "] = {\n")
        for j in range(layers_dim[i + 1]):
            if j < layers_dim[i + 1] - 1:
                header.write(f"{random.uniform(-0.1, 0.1)}, ")
            else:
                header.write(f"{random.uniform(-0.1, 0.1)}")
        header.write("\n};\n");
    header.close();

    print(f"Scrittura di {header_path} completata")
    print(f"Numero di pesi totali: {sum(layers_dim[i] * layers_dim[i + 1] for i in range(len(layers_dim) - 1)) + sum(layers_dim[1:])}") 