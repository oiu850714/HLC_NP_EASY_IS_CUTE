cli: 
packet 0:
    "0" + "\n" + "0" + "\n" + "filename" + "\n"
packet 1 ~ (file size)/best_udp_seg_size + 1:
    "pack_num" + "\n" + "payload len" + "\n" + payload
last packet:
    "INT64_MAX" + "\n"


serv:
packet 0:
    "pack_num + 1"(in this case, 1) + "\n"
packet 1 ~ (file size)/best_udp_seg_size + 1:
    "pack_num + 1" + "\n"
last packet:
    "Bye!\n"

timeout:
    stop_and_wait LOL
