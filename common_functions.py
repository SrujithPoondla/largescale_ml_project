import torch
import numpy as np
import pickle as pc


def get_shape(model):
    shapes = []
    for param in list(model.parameters()):
        shapes.append(list(param.size()))
    return shapes


def push_params_redis(model,db):
    i = -1
    for param in list(model.parameters()):
        i = i+1
        param_data = param.data.numpy()
        p = pc._dumps(param_data, protocol=pc.HIGHEST_PROTOCOL)
        db.set(i, p)


def push_params_memcache(model, client):
    i = -1
    for param in list(model.parameters()):
        i = i + 1
        param_data = param.data.numpy()
        p = pc._dumps(param_data, protocol=pc.HIGHEST_PROTOCOL)
        client.set(str(i), p)


def get_params_redis(db, shapes):
    i = -1
    params=[]
    for shape in shapes:
        i = i + 1
        param = db.get(i)
        param_np = pc._loads(param).reshape(shape)
        param_tensor = torch.nn.Parameter(torch.from_numpy(param_np))
        params.append(param_tensor)
    return params


def get_params_memcache(client, shapes):
    i = -1
    params=[]
    for shape in shapes:
        i = i + 1
        param = client.get(str(i))
        param_np = pc._loads(param).reshape(shape)
        param_tensor = torch.nn.Parameter(torch.from_numpy(param_np))
        params.append(param_tensor)
    return params


def set_params(model,params):
    for param, new_param in zip(list(model.parameters()),params):
        param.data = new_param.data
