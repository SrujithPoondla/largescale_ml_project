import time
start = time.time()

import numpy as np
from scipy.sparse import rand as sprand
import torch
from torch.autograd import Variable


n_users = 1000
n_items = 1000
ratings = sprand(n_users,n_items,density=0.1,format='csr',random_state=42)
ratings.data = (np.random.randint(1,5,size=ratings.nnz).astype(np.float64))
ratings = ratings.toarray() #this is the densification step!

class MatrixFactorization(torch.nn.Module):
    def __init__(self, n_users, n_items, n_factors=20):
        super().__init__()
        self.user_factors = torch.nn.Embedding(n_users,n_factors,sparse=True)
        self.item_factors = torch.nn.Embedding(n_items,n_factors,sparse=True)
        
    def forward(self, user, item):
        return(self.user_factors(user)*self.item_factors(item)).sum(1)
        

model = MatrixFactorization(n_users,n_items,n_factors=20)

loss_func = torch.nn.MSELoss()

optimizer = torch.optim.SGD(model.parameters(),lr=1e-6)

rows, cols = ratings.nonzero()
p = np.random.permutation(len(rows)) #shuffle
rows, cols = rows[p], cols[p]

for row,col in zip(*(rows,cols)):
    rating = Variable(torch.FloatTensor([ratings[row,col]]))
    row = Variable(torch.LongTensor([np.long(row)]))
    col = Variable(torch.LongTensor([np.long(col)]))
    
    prediction = model(row,col)
    loss = loss_func(prediction, rating)
    
    loss.backward()
    
    optimizer.step()

end = time.time()
print(end - start)