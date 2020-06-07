import torch
from model import SCNN
import onnx

input_size = (800, 288)
FILE_NAME = 'scnn_pytorch_half.onnx'

net = SCNN(input_size=input_size, pretrained=False, test=True)
state = torch.load('D:/DHH/Downloads/exp10_best.pth')
net.load_state_dict(state['net'])
net.eval().cuda().half()
input_size = torch.randn(1, 3, 288, 800, device='cuda').half()
torch.onnx.export(net, input_size, FILE_NAME, export_params=True, opset_version=11, do_constant_folding=True)

model = onnx.load(FILE_NAME)
onnx.checker.check_model(model, full_check=True)
print("==> Passed")

output =[node.name for node in model.graph.output]

input_all = [node.name for node in model.graph.input]
input_initializer =  [node.name for node in model.graph.initializer]
net_feed_input = list(set(input_all)  - set(input_initializer))

print('Inputs: ', net_feed_input)
print('Outputs: ', output)
