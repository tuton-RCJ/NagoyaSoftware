import sensor
import time
import ml
from ml.utils import NMS
import math
import image
from pyb import UART
import ustruct
from machine import LED
from collections import deque

sensor.reset()
sensor.set_pixformat(sensor.RGB565)
sensor.set_framesize(sensor.QQVGA)
sensor.set_auto_exposure(True)
sensor.skip_frames(time=2000)
clock = time.clock()

# 各種変数
# debug?
is_debug = True
# 初期化
direction = -1
# 露光時間 (ms)
silver_exposure = 4000
green_exposure = 15000
black_exposure = 15000
red_exposure = 20000
# 黒、緑、赤に対する閾値　(L_low,L_hi,A_low,A_hi,B_low,B_hi)
thre_black = [(30, 70, -15, 10, -10, 25)]
thre_green = [(0, 90, -40, -10, -20, 20)]
thre_red = [(40, 60, 35, 70, 10, 60)]
#　FOMO モデルの設定もろもろ
min_confidence = 0.8
threshold_list = [(math.ceil(min_confidence * 255), 255)]
model = ml.Model("trained")
# rect/circle の色指定用
colors = [
	(255, 0, 0),
	(0, 255, 0),
	(255, 255, 0),
	(0, 0, 255),
	(255, 0, 255),
	(0, 255, 255),
	(255, 255, 255),
]
#　サブマイコンとの UART
uart = UART(3,115200)
uart.init(115200)
# 点群保存用
old_points = deque([],200)
all_points = []
# LED
led = LED("LED_BLUE")
led.on()
start_flag = True
mod = 1 << 8
frame_cnt = 0
frame_disp = deque([],10)

# bit tuple 用関数
def bit_tuple(x,y,z):
	return (x << 16) + (y << 8) + z
def bit_tie(x):
	return x >> 16, (x >> 8)% mod, x % mod

# UART するときのラッパー value:int
def send(value):
	uart.write(ustruct.pack('B',value))

# FOMO モデルの出力パース関数
def fomo_post_process(model, inputs, outputs):
	n, oh, ow, oc = model.output_shape[0]
	nms = NMS(ow, oh, inputs[0].roi)
	for i in range(oc):
		img = image.Image(outputs[0][0, :, :, i] * 255)
		blobs = img.find_blobs(
			threshold_list, x_stride=1, area_threshold=1, pixels_threshold=1
		)
		for b in blobs:
			rect = b.rect()
			x, y, w, h = rect
			score = (
				img.get_statistics(thresholds=threshold_list, roi=rect).l_mean() / 255.0
			)
			nms.add_bounding_box(x, y, x + w, y + h, score, i)
	return nms.get_bounding_boxes()

# デバッグ用 value: print-able
def debug_print(value):
	if is_debug:
		print(value)

# dbscan
def dbscan(points,min_pts = 3):
	eps = 40	  # eps: この距離以下なら近傍とみなす
	n = len(points)
	visited = [False] * n
	labels = [None] * n
	cluster_id = 0

	def region_query(idx):
		neighbors = []
		p = points[idx]
		t1, x1, y1 = bit_tie(p)
		for j in range(n):
			q = points[j]
			t2, x2, y2 = bit_tie(q)
			dx = x1 - x2
			dy = y1 - y2
			dist = (dx * dx + dy * dy) ** 0.5
			if dist <= eps:
				neighbors.append(j)
		return neighbors

	for i in range(n):
		if visited[i]:
			continue
		visited[i] = True
		neighbors = region_query(i)
		if len(neighbors) < min_pts:
			labels[i] = -1
		else:
			cluster_id += 1
			labels[i] = cluster_id
			seed_set = neighbors[:]
			while seed_set:
				j = seed_set.pop(0)
				if not visited[j]:
					visited[j] = True
					j_neighbors = region_query(j)
					if len(j_neighbors) >= min_pts:
						for neigh in j_neighbors:
							if neigh not in seed_set:
								seed_set.append(neigh)
				if labels[j] is None:
					labels[j] = cluster_id
	clusters_xmax = {}
	mod_t = (5 if direction == 3 else 10)
	for idx, lab in enumerate(labels):
		if lab not in clusters_xmax:
			clusters_xmax[lab] = points[idx]
		t1, x1, y1 = bit_tie(points[idx])
		t2, x2, y2 = bit_tie(clusters_xmax[lab])
		if (t1-frame_cnt)%mod_t < (t2-frame_cnt)%mod_t:
			clusters_xmax[lab] = points[idx]
	return clusters_xmax

# 銀被災者検知
def detect_silver():
	while sensor.get_exposure_us() != silver_exposure:
		sensor.set_auto_exposure(False,exposure_us=silver_exposure)
	img = sensor.snapshot().lens_corr(1.8)
	result = []
	for i, detection_list in enumerate(model.predict([img], callback=fomo_post_process)):
		if i == 0:
			continue  # background class
		if len(detection_list) == 0:
			continue  # no detections for this class?
		for (x, y, w, h), score in detection_list:
			center_x = math.floor(x + (w / 2))
			center_y = math.floor(y + (h / 2))
			img.draw_circle((center_x, center_y, 12), color=colors[0])
			debug_print(f"x {center_x}\ty {center_y}\tscore {score}")
			result.append(bit_tuple(frame_cnt,center_x,center_y))
	return result,img

# 緑ゾーン検知
def detect_green():
	while sensor.get_exposure_us() != green_exposure:
		sensor.set_auto_exposure(False,exposure_us=green_exposure)
	img = sensor.snapshot().lens_corr(1.8)
	result = []
	for obj in img.find_blobs(thre_green,area_threshold=400,merge=True):
		img.draw_rectangle(obj[:4],colors[1])
		result.append(bit_tuple(frame_cnt,obj[0]+obj[2]//2,obj[1]+obj[3]//2))
	return result,img

# 黒被災者検知
def detect_black():
	while sensor.get_exposure_us() != black_exposure:
		sensor.set_auto_exposure(False,exposure_us=black_exposure)
	img = sensor.snapshot().lens_corr(1.8)
	result = []
	for o in img.find_blobs(thre_black,area_threshold=80):
		img.draw_rectangle(o[:4],colors[2])
		if o[2]+o[3] > 100:
			continue
		if abs(o[2]-o[3]) <= 4:
			img.to_grayscale()
			if  100 < img.get_statistics(roi=(o[0],o[1],o[2],o[3])).mean() < 160 and o.roundness() > 0.60:
				img.draw_rectangle(o[:4],colors[4])
				result.append(bit_tuple(frame_cnt,o[0]+o[2]//2,o[1]+o[3]//2))
			print(img.get_statistics(roi=(o[0],o[1],o[2],o[3])).mean(), o.roundness())
	return result,img

#　赤ゾーン検知
def detect_red():
	while sensor.get_exposure_us() != red_exposure:
		sensor.set_auto_exposure(False,exposure_us=red_exposure)
	clock.tick()
	img = sensor.snapshot().lens_corr(1.8)
	result = []
	for obj in img.find_blobs(thre_red,area_threshold=400,merge=True):
		result.append(bit_tuple(frame_cnt,obj[0]+obj[2]//2,obj[1]+obj[3]//2))
		img.draw_rectangle(obj[:4],color=colors[5])
	return result,img

# 点群の処理と、UART
def process_points(points,different_flag,direction):
	global old_points, all_points
	if different_flag:
		old_points = deque([],100)
		all_points = []
	if len(old_points) >= (5 if direction == 0 else 10):
		del_points = old_points.popleft()
		for p in del_points:
			all_points.remove(p)
	old_points.append(points)
	all_points.extend(points)
	return dbscan(all_points,min_pts = (3 if  direction == 0 else 7))

# メインルーチンで関数を呼び出す用
def detect(direction):
	if direction == 0:
		return detect_silver()
	elif direction == 1:
		return detect_green()
	elif direction == 2:
		return detect_black()
	elif direction == 3:
		return detect_red()


while True:
	led.on()
	different_flag = False
	while uart.any() == 0 and start_flag:
		pass
	if uart.any() != 0:
		different_flag = True
		frame_cnt = 0
		direction = int(uart.readchar())
	frame_cnt %= (5 if direction == 0 else 10)
	clock.tick()
	debug_print(direction)
	points,img = detect(direction)
	res = process_points(points,different_flag,direction)
	min_x = 255
	for k,r in res.items():
		if k == -1:
			continue
		t, x, y = bit_tie(r)
		min_x = min(x,min_x)
		img.draw_line(x,0,x,120,color=colors[4])
	send(min_x)
	print(res)
	uart.flush()
	start_flag = False
	led.off()
	frame_cnt += 1
	print(clock.fps(),sensor.get_exposure_us())