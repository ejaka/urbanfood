from matplotlib import pyplot as plt


time_x= [20,40,60,80,100,130,150,180,200,250,300,310]

throughput_y = [72.422,74.4867,75.1517,75.4799,75.6759,75.8554,75.935,76.0212,76.064,76.1411,76.1926,76.2008]


plt.plot(time_x,throughput_y )

plt.xlabel('time')
plt.ylabel('throughput')

plt.title('time against distnace')

plt.show()
