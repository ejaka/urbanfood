from matplotlib import pyplot as plt

"distance of 2m"
time_x = [10,20,40,55,70,90,110,125,150,170]

throughput_y = [67.9499,72.422,74.4873,75.032,75.3401,75.589,75.7469,75.8315,75.9349,75.9959,]



plt.plot(time_x,throughput_y )


plt.xlabel('run time in sec')
plt.ylabel('throughput in Mbps')

plt.title('preliminary simulation experiment at 2m')

plt.legend(["TwoRayProp"])

plt.show()

