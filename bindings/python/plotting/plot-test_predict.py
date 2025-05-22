import numpy as np 
import pandas as pd
import pickle
import matplotlib.pyplot as plt
import statsmodels.api as sm

with open('bindings/python/results/trained_model.pkl', 'rb') as f:
    model, feature_names = pickle.load(f)

data = pd.read_csv('bindings/python/results/sampled_configurations.csv')

for feature in feature_names:
    if '$$' in feature:
        parts = feature.split('$$')
        if all(part in data.columns for part in parts):
            data[feature] = data[parts[0]] * data[parts[1]]
        else:
            print(f"Skipping {feature} because missing parts: {parts}")
    else:
        if feature not in data.columns:
            print(f"Missing simple feature {feature}. Setting to 0.")
            data[feature] = 0

X = data[feature_names]
X = sm.add_constant(X, has_constant='add')
print(X)
predictions = model.predict(X)

plt.figure(figsize=(12, 6))
plt.bar(range(len(predictions)), predictions, color='blue', alpha=0.7)
plt.xlabel('Config Number')
plt.ylabel('Predicted Performance')
plt.title('Distribution of Predicted Performance')
plt.grid(True)
plt.tight_layout()
plt.show()