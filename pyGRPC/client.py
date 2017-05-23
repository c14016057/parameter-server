from __future__ import print_function
import numpy as np
from keras.datasets import mnist
from keras.models import Sequential
from keras.layers.core import Dense, Dropout, Activation
from keras.utils import np_utils


import grpc
import keyvector_pb2
import keyvector_pb2_grpc

nb_classes = 10
(X_train, y_train), (X_test, y_test) = mnist.load_data()

X_train = X_train.reshape(60000, 784)
X_test = X_test.reshape(10000, 784)
X_train = X_train.astype('float32')
X_test = X_test.astype('float32')
X_train /= 255
X_test /= 255

Y_train = np_utils.to_categorical(y_train, nb_classes)
Y_test = np_utils.to_categorical(y_test, nb_classes)


MAX_KEY = 5000
MAX_VALUE = 1000

channel = grpc.insecure_channel('140.112.30.241:50051', [('grpc.max_receive_message_length', 100 * 1024 * 1024)])
stub = keyvector_pb2_grpc.pushPullRequestStub(channel)

def push(sendBuffer, numP):
    
    msg = keyvector_pb2.keyVectorMessage()
    for i in range(0, MAX_KEY):
        msg.key.append(i)
    for i in range(numP):
        msg.val.append(sendBuffer[i])
    msg.nump = numP
    
    msg.iter = 1
    
    response = stub.push(msg)
    
    print('Pushed.\n')





#######model########





model = Sequential()
model.add(Dense(512, input_shape=(784,)))
model.add(Activation('relu')) 
model.add(Dropout(0.2))
model.add(Dense(512))
model.add(Activation('relu'))
model.add(Dropout(0.2))
model.add(Dense(10))
model.add(Activation('softmax'))
model.summary()
model.compile(loss='categorical_crossentropy', optimizer='adam', metrics=['accuracy'])

for i in range(5):
    model.fit(X_train, Y_train,batch_size=128, epochs=1, verbose=1,validation_data=(X_test, Y_test))
    
    
    sendBuffer = []
    for layer in model.layers:
        h=layer.get_weights()
        if(len(h)>0):
            for i in range(len(h[0])):
                for j in range(len(h[0][i])):
                    sendBuffer.append(h[0][i][j])
            for i in range(len(h[1])):
                sendBuffer.append(h[1][i])
    numP = len(sendBuffer)
    push(sendBuffer, numP)
    
    
    msg = keyvector_pb2.keyMessage()
    for i in range(0, MAX_KEY):
        msg.key.append(i)
    msg.nump = numP
    response = stub.pull(msg)
    sendBuffer = response.val

    layer0_Weight = sendBuffer[0:401408]
    layer0_Weight = np.asarray(layer0_Weight).reshape(784, 512)
    layer0_Bias = sendBuffer[401408:401920]
    layer0_Bias = np.asarray(layer0_Bias).reshape(512)
    model.layers[0].set_weights([layer0_Weight, layer0_Bias])

    layer3_Weight = sendBuffer[401920:664064]
    layer3_Weight = np.asarray(layer3_Weight).reshape(512, 512)
    layer3_Bias = sendBuffer[664064:664576]
    layer3_Bias = np.asarray(layer3_Bias).reshape(512)
    model.layers[3].set_weights([layer3_Weight, layer3_Bias])

    layer6_Weight = sendBuffer[664576:669696]
    layer6_Weight = np.asarray(layer6_Weight).reshape(512, 10)
    layer6_Bias = sendBuffer[669696:669706]
    layer6_Bias = np.asarray(layer6_Bias).reshape(10)
    model.layers[6].set_weights([layer6_Weight, layer6_Bias])
