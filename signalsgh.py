from matplotlib import pyplot as plt

distance_x= [1,5,10,20,40,55,70,90,110,125,150,170,200]
signalFriisProp_y= [-34.6839,-48.6633,-54.6839,-60.7045,-66.7251,-69.4912,-71.5859,-73.7688,-75.5118,-76.6221,-78.2058,-79.2929,-80.7045]


signalFixedProp_y= [-78,-78,-78,-78,-78,-78,-78,-78,-78,-78,-78,-78,-78]

signalThreelogProp_y=[-34.6777,-47.9581,-53.6777,-59.3973,-65.1168,-67.7446,-69.7346,-71.8083,-73.4642,-74.519,-76.0234,-77.0562,-78.3973]

signalTwoRayProp_y= [-34.6839,-48.6633,-54.6839,-60.7045,-66.7251,-69.6557,-73.8451,-78.2109,-81.6969,0,0,0,0]

signalNakagamiProp_y= [10.3923,10.3924,10.3925,10.3924,10.3922,10.392,10.392,8.53507, 8.53526,8.53487,8.53463,8.53461,8.53478]
plt.plot(distance_x,signalFriisProp_y)

plt.plot(distance_x,signalFixedProp_y)

plt.plot(distance_x,signalThreelogProp_y)

plt.plot(distance_x,signalTwoRayProp_y)

plt.plot(distance_x,signalNakagamiProp_y)

plt.xlabel('distance in meter')
plt.ylabel('Signal strength in dBm')

plt.title('Test for Signal Strength')

plt.legend(["FriisProp", "FixedProp","ThreelogProp","TwoRayProp","NakagamiProp"])


plt.show()
