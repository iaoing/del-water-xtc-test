# -*- coding: utf-8 -*-
"""
Created on 2017/5/23 14:45:40

@author bing Jiao
"""
import matplotlib.pyplot as plt
import numpy as np
import math

def get_coord():
	bytecnt, realBegin, realEnd, caluBegin, caluEnd = [], [], [], [], []
	fd = open("../origin-info/waterNo.txt", 'r')

	while(1):
		line = fd.readline()
		if line:
			if line[0] != '#':
				bytecnt.append(int(line[0:11]))
				realBegin.append(int(line[11:21]))
				realEnd.append(int(line[21:31]))
				caluBegin.append(int(line[31:41]))
				caluEnd.append(int(line[41:]))
				# print(bytecnt, realBegin, realEnd, caluBegin, caluEnd)
				# break
			else:
				pass
		else:
			break

	fd.close()

	return bytecnt, realBegin, realEnd, caluBegin, caluEnd

# can't use it, some error
def nihe(x, y):
	x_son = x[0:50]
	y_son = y[0:50]
	z1 = np.polyfit(x_son, y_son, 3) #用3次多项式拟合c
	p1 = np.poly1d(z1)
	# print(p1) #在屏幕上打印拟合多项式
	yvals=p1(x) #也可以使用yvals=np.polyval(z1,x)
	plot1=plt.plot(x, y, '*',label='original values')
	plot2=plt.plot(x, yvals, 'r',label='polyfit values')
	plt.xlabel('x axis')
	plt.ylabel('y axis')
	plt.legend(loc=4) #指定legend的位置,读者可以自己help它的用法
	plt.title('  ')
	plt.show()


def plot_it(x, y):
	plot = plt.plot(x, y, '*', label='###')
	plt.xlabel('count')
	plt.ylabel('position (byte)')
	plt.legend(loc='upper right')
	plt.show()


def plot_these(x, yList):
	plot = []
	marker = ['*', 'v', '^']
	for i in range(len(yList)):
		plot.append(plt.plot(x, yList[i], marker[i]))
	plt.xlabel('x axis')
	plt.ylabel('y axis')
	plt.legend(loc=4) #指定legend的位置,读者可以自己help它的用法
	plt.title('###')
	plt.show()
	
def write_file(filename, title, *alist):
	fd = open(filename, 'w')
	# buf = "#detBytecnt, detRealBegin, detRealEnd, detCaluBegin, detCaluEnd\n"
	buf = title;
	fd.write(buf)
	for i in range(len(alist[0])):
		buf = ""
		for j in range(len(alist)):
			buf += "%10d," % (alist[j][i])
		buf = buf.rstrip(',')
		buf += '\n'
		fd.write(buf);
	fd.close()




if __name__ == '__main__':
	
	# get data
	bytecnt, realBegin, realEnd, caluBegin, caluEnd = get_coord()
	

	#############################################
	# init det
	"""
	###### these det maybe error, because we should sub first frame but not pre frame for per frame!
	detBytecnt, detRealBegin, detRealEnd, detCaluBegin, detCaluEnd = [], [], [], [], []
	for i in range(len(bytecnt) - 1):
		detBytecnt.append(bytecnt[i+1] - bytecnt[i])
		detRealBegin.append(realBegin[i+1] - realBegin[i])
		detRealEnd.append(realEnd[i+1] - realEnd[i])
		detCaluBegin.append(caluBegin[i+1] - caluBegin[i])
		detCaluEnd.append(caluEnd[i+1] - caluEnd[i])

	# write_file("./detInfo.txt", \
		# "#detBytecnt, detRealBegin, detRealEnd, detCaluBegin, detCaluEnd\n", \
		# detBytecnt, detRealBegin, detRealEnd, detCaluBegin, detCaluEnd)

	"""
	# init det
	#############################################
	


	#############################################
	# avg of det
	"""

	tmp = [abs(x) for x in detBytecnt[0:10]]
	sum10BC = sum(tmp[0:10])
	tmp = [abs(x) for x in detRealBegin[0:10]]
	sum10RB = sum(tmp[0:10])
	tmp = [abs(x) for x in detRealEnd[0:10]]
	sum10RE = sum(tmp[0:10])

	avg10RB = sum10RB/sum10BC
	avg10RE = sum10BC/sum10BC
	print(avg10RB, avg10RE)

	pct5list10 = []
	for i in range(10):
		pct5list10.append(realEnd[i] - realBegin[i])
	pct5 = round((sum(pct5list10))/10/200)	# 0.5% in begin and end

	baseBytecnt = bytecnt[0]
	baseRealBegin = realBegin[0]
	baseRealEnd = realEnd[0]
	caluAddAvgBegin, caluAddAvgEnd = [], []
	caluAddAvgBegin.append(baseRealBegin)
	caluAddAvgEnd.append(baseRealEnd)
	caluPct5Begin, caluPct5End = [], []
	caluPct5Begin.append(baseRealBegin)
	caluPct5End.append(baseRealEnd)
	for i in range(len(bytecnt) - 1):
		caluAddAvgBegin.append(math.floor(baseRealBegin + (bytecnt[i+1] - bytecnt[i]) * avg10RB))
		caluAddAvgEnd.append(math.ceil(baseRealEnd   + (bytecnt[i+1] - bytecnt[i]) * avg10RE))
		caluPct5Begin.append(round(baseRealBegin + (bytecnt[i+1] - bytecnt[i]) * avg10RB + pct5))
		caluPct5End.append(round(baseRealEnd + (bytecnt[i+1] - bytecnt[i]) * avg10RE - pct5))
	write_file("./caluAvg.txt", \
		"# avg10RB: %f      avg10RE: %f     pct5: %f\
		\n# bytecnt, realBegin, caluAddAvgBegin, caluPct5Begin, realEnd, caluAddAvgEnd, caluPct5End\n" \
		% (avg10RB, avg10RE, pct5), \
		bytecnt, realBegin, caluAddAvgBegin, caluPct5Begin, realEnd, caluAddAvgEnd, caluPct5End)


	tmp1, tmp2, tmp3, tmp4 = [], [], [], []
	for i in range(len(caluAddAvgBegin)):
		if i == 0:
			continue
		if (caluPct5Begin[i] == realBegin[i]) or (realEnd[i] == caluPct5End[i]):
			print("%d  %10d%10d%10d%10d\n" % (i, caluPct5Begin[i], realBegin[i], realEnd[i], caluPct5End[i]))
			continue
		tmp1.append(caluAddAvgBegin[i] - realBegin[i])
		tmp2.append(realEnd[i] - caluAddAvgEnd[i])
		tmp3.append(caluPct5Begin[i] - realBegin[i])
		tmp4.append(realEnd[i] - caluPct5End[i])

	print(max(tmp1), min(tmp1))
	print(max(tmp2), min(tmp2))
	print(max(tmp3), min(tmp3))
	print(max(tmp4), min(tmp4))

	"""
	# avg of det
	#############################################
	


	#############################################
	# one byte per hundred atome(exclude water) calu the water index
	"""
	############################# maybe error ! 
	# some test for get std
	# print(max(realBegin), min(realBegin))	# wBeginAdd std
	# realWaterByte = []
	# for i in range(len(realBegin)):
	# 	realWaterByte.append(realEnd[i] - realBegin[i])
	# print(max(realWaterByte), min(realWaterByte))

	std		= 0.01
	stdW	= 0.001
	nAtoms	= 45608
	wBegin 	= 19297
	wEnd 	= 45561

	wBeginAdd	= wBegin * std				# frame begin ---- water begin  *  0.01
	wWaterAdd	= (wEnd - wBegin) * stdW	# water begin ---- water end    *  0.001

	baseBytecnt 	= bytecnt[0]
	baseRealBegin	= realBegin[0]
	baseRealEnd 	= realEnd[0]
	baseResidue		= bytecnt[0] - realEnd[0]
	baseWater		= realEnd[0] - realBegin[0]
	
	caluPerBegin, caluPerEnd = [], []
	caluPerBegin.append(realBegin[0])
	caluPerEnd.append(realEnd[0])

	
	for i in range(1, len(realBegin)):
		caluPerBegin.append(math.ceil(baseRealBegin + detBytecnt[i-1] + wBeginAdd))
		# caluPerEnd.append(math.floor(baseRealBegin + detBytecnt[i-1] - wBeginAdd + baseWater))
		caluPerEnd.append(math.floor(baseRealEnd + detBytecnt[i-1] - wBeginAdd))

	title = "# %10f%10f\n" % (wBeginAdd, wWaterAdd)
	title += "# bytecnt, realBegin, caluPerBegin, realEnd, caluPerEnd\n"
	write_file("./caluPerWater.txt", title, bytecnt, realBegin, caluPerBegin, realEnd, caluPerEnd)



	# print(max(realBegin), (min(realBegin)))
	# print(max(caluPerBegin), (min(caluPerBegin)))
	# print(max(realEnd), (min(realEnd)))
	# print(max(caluPerEnd), (min(caluPerEnd)))

	print(wBeginAdd, wWaterAdd)
	detTmp1, detTmp2 = [], []
	for i in range(1, len(realBegin)):
		detTmp1.append(caluPerBegin[i] - realBegin[i])
		detTmp2.append(realEnd[i] - caluPerEnd[i])
		# if(realEnd[i] <= caluPerEnd[i]):
		# 	print(i)
	print(max(detTmp1), (min(detTmp1)))
	print(max(detTmp2), (min(detTmp2)))


	"""
	# one byte per hundred atome(exclude water) calu the water index
	#############################################
	



	#############################################
	# caluDetType1
	"""

	# all basexxxx are come from first frame
	baseBytecnt 	= bytecnt[0]
	baseRealBegin 	= realBegin[0]
	baseRealEnd 	= realEnd[0]
	# all these det are come from taht sub first frame for per frame
	detBytecnt, detRealBegin, detRealEnd = [], [], []
	for i in range(len(bytecnt)):
		detBytecnt.append(bytecnt[i]	 - baseBytecnt)
		detRealBegin.append(realBegin[i] - baseRealBegin)
		detRealEnd.append(realEnd[i] 	 - baseRealEnd)

	caluDetBegin, caluDetEnd = [], []
	for i in range(len(bytecnt)):
		if detBytecnt[i] == 0:
			caluDetBegin.append(baseRealBegin + 1)
			caluDetEnd.append(baseRealEnd - 1)
		elif detBytecnt[i] < 0:
			caluDetBegin.append(baseRealBegin)
			caluDetEnd.append(baseRealEnd - detBytecnt[i])
		else:
			caluDetBegin.append(baseRealBegin + detBytecnt[i])
			caluDetEnd.append(baseRealEnd)

	# title = "# bytecnt, realBegin, caluDetBegin, realEnd, caluDetEnd\n"
	# write_file("./caluDetType1.txt", title, bytecnt, realBegin, caluDetBegin, realEnd, caluDetEnd)

	# test
	# detTmp1, detTmp2 = [], []
	# for i in range(1, len(realBegin)):
	# 	detTmp1.append(caluDetBegin[i] - realBegin[i])
	# 	detTmp2.append(realEnd[i] - caluDetEnd[i])
	# print(max(detTmp1), (min(detTmp1)))
	# print(max(detTmp2), (min(detTmp2)))
	# detTmp1.append(0)
	# detTmp1.sort()
	# neg = detTmp1.index(0)
	# zer = detTmp1.count(0) - 1
	# pos = len(detTmp1) - neg - zer
	# print(neg, zer, pos)
	# detTmp2.append(0)
	# detTmp2.sort()
	# neg = detTmp2.index(0)
	# zer = detTmp2.count(0) - 1
	# pos = len(detTmp2) - neg - zer
	# print(neg, zer, pos)

	"""
	# caluDetType1
	#############################################



	#############################################
	# caluDetType2

	std		= 0.004
	stdW	= 0.004
	nAtoms	= 45608
	wBegin 	= 19297
	wEnd 	= 45561

	wBeginAdd	= wBegin * std				# frame begin ---- water begin  *  0.004
	wWaterAdd	= (wEnd - wBegin) * stdW	# water begin ---- water end    *  0.004

	wBeginAdd = math.ceil(wBeginAdd)
	wWaterAdd = math.ceil(wWaterAdd)
	print(wBeginAdd, wWaterAdd)

	# all basexxxx are come from first frame
	baseBytecnt 	= bytecnt[0]
	baseRealBegin 	= realBegin[0]
	baseRealEnd 	= realEnd[0]
	# all these det are come from taht sub first frame for per frame
	detBytecnt, detRealBegin, detRealEnd = [], [], []
	for i in range(len(bytecnt)):
		detBytecnt.append(bytecnt[i]	 - baseBytecnt)
		detRealBegin.append(realBegin[i] - baseRealBegin)
		detRealEnd.append(realEnd[i] 	 - baseRealEnd)

	caluDetBegin, caluDetEnd = [], []
	for i in range(len(bytecnt)):
		if detBytecnt[i] == 0:
			caluDetBegin.append(baseRealBegin + wBeginAdd)
			caluDetEnd.append(baseRealEnd - wBeginAdd - wWaterAdd)
		elif detBytecnt[i] < 0:
			caluDetBegin.append(baseRealBegin - detBytecnt[i] + wBeginAdd)
			caluDetEnd.append(baseRealEnd - detBytecnt[i] - wBeginAdd - wWaterAdd)
		else:
			caluDetBegin.append(baseRealBegin + detBytecnt[i] + wBeginAdd)
			caluDetEnd.append(baseRealEnd + detBytecnt[i] - wBeginAdd - wWaterAdd)

	title = "# bytecnt, realBegin, caluDetBegin, realEnd, caluDetEnd\n"
	write_file("./caluDetType2.txt", title, bytecnt, realBegin, caluDetBegin, realEnd, caluDetEnd)

	# test
	detTmp1, detTmp2 = [], []
	for i in range(1, len(realBegin)):
		detTmp1.append(caluDetBegin[i] - realBegin[i])
		detTmp2.append(realEnd[i] - caluDetEnd[i])
	print(max(detTmp1), (min(detTmp1)))
	print(max(detTmp2), (min(detTmp2)))
	detTmp1.append(0)
	detTmp1.sort()
	neg = detTmp1.index(0)
	zer = detTmp1.count(0) - 1
	pos = len(detTmp1) - neg - zer
	print(neg, zer, pos)
	detTmp2.append(0)
	detTmp2.sort()
	neg = detTmp2.index(0)
	zer = detTmp2.count(0) - 1
	pos = len(detTmp2) - neg - zer
	print(neg, zer, pos)

	
	# caluDetType2
	#############################################
