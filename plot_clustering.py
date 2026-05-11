import matplotlib.pyplot as plt
import numpy as np

n_values = [5, 5, 5, 7, 7, 10]
k_values = [2, 3, 4, 2, 3, 2]
times = [16.0, 0.0, 6.0, 0.0, 6.0, 12.0]

plt.figure(figsize=(10, 6))

unique_n = sorted(set(n_values))
for k in sorted(set(k_values)):
    n_k = [n for n, k2 in zip(n_values, k_values) if k2 == k]
    t_k = [t for n, k2, t in zip(n_values, k_values, times) if k2 == k]
    if n_k:
        plt.plot(n_k, t_k, 'o-', label=f'k = {k}', linewidth=2, markersize=8)

plt.xlabel('Number of elements (n)', fontsize=12)
plt.ylabel('Time (ms)', fontsize=12)
plt.title('Clustering Time vs Dataset Size', fontsize=14)
plt.legend(fontsize=11)
plt.grid(True, alpha=0.3)

n_extrap = [10, 12, 14, 16, 18, 20]
time_extrap = [12 * (2**(n-10)) for n in n_extrap]
plt.plot(n_extrap, time_extrap, 'r--', linewidth=1.5, alpha=0.7, label='Extrapolation (k=2)')

plt.yscale('log')
plt.savefig('clustering_time_plot.png', dpi=150)
plt.show()

print("\nTime table:")
print(f"{'N':<6} {'K':<6} {'Time (ms)':<12}")
print("-" * 25)
for n, k, t in zip(n_values, k_values, times):
    print(f"{n:<6} {k:<6} {t:<12.3f}")

print("\nExtrapolation (k=2):")
for n, t in zip(n_extrap, time_extrap):
    print(f"N={n:<3} -> {t:.0f} ms ({t/1000:.1f} sec)")