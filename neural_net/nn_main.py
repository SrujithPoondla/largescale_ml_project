from __future__ import print_function

import argparse
import asyncio
import timeit

import torch
import torch.multiprocessing as mp
import torch.nn as nn
import torch.nn.functional as F

from common_functions import push_params_redis, get_shapes_len
from neural_net.nn_train_plain import train_plain
from neural_net.nn_train_redis import train_redis

# Training settings

parser = argparse.ArgumentParser(description='PyTorch MNIST Example')
parser.add_argument('--disable_redis', dest='is_redis', action='store_false')
parser.add_argument('--batch-size', type=int, default=64, metavar='N',
                    help='input batch size for training (default: 64)')
parser.add_argument('--test-batch-size', type=int, default=1000, metavar='N',
                    help='input batch size for testing (default: 1000)')
parser.add_argument('--epochs', type=int, default=10, metavar='N',
                    help='number of epochs to train (default: 10)')
parser.add_argument('--lr', type=float, default=0.01, metavar='LR',
                    help='learning rate (default: 0.01)')
parser.add_argument('--momentum', type=float, default=0.5, metavar='M',
                    help='SGD momentum (default: 0.5)')
parser.add_argument('--seed', type=int, default=1, metavar='S',
                    help='random seed (default: 1)')
parser.add_argument('--log-interval', type=int, default=10, metavar='N',
                    help='how many batches to wait before logging training status')
parser.add_argument('--num-processes', type=int, default=1, metavar='N',
                    help='how many training processes to use (default: 2)')


class Net(nn.Module,):
    def __init__(self):
        super(Net, self).__init__()
        self.conv1 = nn.Conv2d(1, 10, kernel_size=5)
        self.conv2 = nn.Conv2d(10, 20, kernel_size=5)
        self.conv2_drop = nn.Dropout2d()
        self.fc1 = nn.Linear(320, 50)
        self.fc2 = nn.Linear(50, 10)

    def forward(self, x):
        x = F.relu(F.max_pool2d(self.conv1(x), 2))
        x = F.relu(F.max_pool2d(self.conv2_drop(self.conv2(x)), 2))
        x = x.view(-1, 320)
        x = F.relu(self.fc1(x))
        x = F.dropout(x, training=self.training)
        x = self.fc2(x)
        return F.log_softmax(x, dim=1)

if __name__ == '__main__':

    # common code
    start_time = timeit.default_timer()
    args = parser.parse_args()
    torch.manual_seed(args.seed)
    model = Net()

    print("Number of proesses:"+ str(args.num_processes))
    if args.is_redis:
        print("Redis")
    else:
        print("Plain HogWild")
    if args.is_redis:
        # db = redis(db=0)
        # db = client.Client(('localhost', 11211))
        # push params to redis cache
        loop = asyncio.get_event_loop()
        push_params_redis(model,loop)
        shapes_len = get_shapes_len(model)
        processes = []
        for rank in range(args.num_processes):
            p = mp.Process(target=train_redis, args=(rank, args, model, shapes_len,loop))
            p.start()
            processes.append(p)
        for p in processes:
            p.join()
    else:
        # push params to memcache
        # push_params_memcache(model, db)
        # shapes = get_shape(model)
        # train(args, model, shapes, db)

        model.share_memory() # gradients are allocated lazily, so they are not shared here

        processes = []
        for rank in range(args.num_processes):
            p = mp.Process(target=train_plain, args=(rank, args, model))
            p.start()
            processes.append(p)
        for p in processes:
            p.join()

    # # your code
    print("Total execution time: {}\n".format(timeit.default_timer() - start_time))
