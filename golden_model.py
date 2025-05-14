import torch
import torch.nn as nn
import torch.nn.functional as F

model_path = "best_agent_noise01.pt"

model_input = torch.tensor([ 1.4067, -0.5529, -0.0704,  0.0493, -0.0883,  0.5657,  0.0858, -0.9593, 0.1078, -0.6169,  1.5449,  0.2281,  0.5531,  0.1447,  0.6435, -1.1590, 0.6531 ]) 

class PolicyNetwork(nn.Module):
    def __init__(self):
        super().__init__()
        self.net = nn.Sequential(
            nn.Linear(17, 256),
            nn.ReLU(),
            nn.Linear(256, 256),
            nn.ReLU(),
            nn.Linear(256, 128),
            nn.ReLU()
        )
        self.mean_layer = nn.Linear(128, 4)
        self.log_std_parameter = nn.Parameter(torch.zeros(4))  # viene caricato dai pesi
        self.value_layer = nn.Linear(128, 1)

    def forward(self, x):
        x = self.net(x)
        mean = self.mean_layer(x)
        log_std = self.log_std_parameter.expand_as(mean)
        value = self.value_layer(x)
        return mean, log_std, value


checkpoint = torch.load(model_path, map_location='cpu')

model = PolicyNetwork()
model.load_state_dict(checkpoint['policy'])
model.eval()

mean, log_std, value = model(model_input)

print("Mean:", mean)
print("Log Std:", log_std)
print("Value:", value)
