import pickle
import matplotlib.pyplot as plt

plt.rcParams['text.usetex'] = False


with open('bindings/python/results/trained_model.pkl', 'rb') as f:
    model, feature_names = pickle.load(f)


safe_names = [name.replace('$', '&') for name in feature_names]

coeffs = model.params.values
intercept = model.params['const'] if 'const' in model.params else None

feature_coef_pairs = list(zip(safe_names, coeffs[1:]))
feature_coef_pairs.sort(key=lambda x:abs(x[1]), reverse=True)

names, values = zip(*feature_coef_pairs)

plt.figure(figsize=(12, 6)) #!!!!
plt.barh(names, values)
plt.xlabel('Measured Performance')
plt.title('Girl, whatever')
plt.gca().invert_yaxis()
plt.grid(True)
plt.tight_layout() #!!!!!

plt.savefig('bindings/python/plotting', dpi=300)
plt.show()