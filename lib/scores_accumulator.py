# ID-Fits
# Copyright (c) 2015 Institut National de l'Audiovisuel, INA, All rights reserved.
# 
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 3.0 of the License, or (at your option) any later version.
# 
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
# 
# You should have received a copy of the GNU Lesser General Public
# License along with this library.


import numpy as np



class ScoresAccumulator:

    def __init__(self, max_size=int(1e3)):
        self.accumulator = {}
        self.max_size = max_size
        self.width = 20
        self.kernel = self._gaussian_kernel(np.arange(self.width, self.width+1), sigma=10.0)


    def _gaussian_kernel(self, r, sigma):
        return np.exp(-0.5 * (r /sigma)**2) / (np.sqrt(2*np.pi)*sigma)

    
    def addScores(self, scores, t):
        for label, score in scores:
            if label not in self.accumulator:
                self.accumulator[label] = np.zeros((self.max_size+2*self.width))
            self.accumulator[label][t:t+2*self.width+1] += score * self.kernel


    def getAccumulator(self):
        return list(self.accumulator.iteritems())


    def getBestLabels(self, tmin=0, tmax=-1):
        if tmax == -1:
            tmax = self.max_size
        
        if len(self.accumulator) == 0:
            return []

        l = list(self.accumulator.iteritems())

        best_labels = []
        for t in range(tmax):
            label, _ = sorted(l, key=lambda x: x[1][t+self.width], reverse=True)[0]
            best_labels.append(label)

        best_labels2 = []
        for label in set(best_labels):
            if best_labels.count(label) >= 8:
                best_labels2.append(label)
                
        return sorted(best_labels2)


    def getLabelsScores(self, labels, tmin=0, tmax=-1):
        if tmax == -1:
            tmax = self.max_size
        return [self.accumulator[label][self.width+tmin:self.width+tmax] for label in labels]
