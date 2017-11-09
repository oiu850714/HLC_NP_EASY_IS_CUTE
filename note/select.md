I/O models
一個 I/O operation 可以被抽象成兩個 phase：
    第一是跟 kernel 發 request，並且等待 kernel 說 request 完成
    第二是在 kernel 說 request 完成後，讀取 data

而你的程式會因為你執行 I/O operation 時使用不同的 I/O model 而有不同的行為
有可能被 block，有可能不會

依照 POSIX 定義[UNP 6.2 最後一頁]，I/O model 可以分為兩大類：
    Synchronous I/O -
        如果你的 I/O operation 內，request phase 會導致你的程式被 block，
        這個 I/O operation 就是屬於 Synchronous I/O
    Asynchronous I/O -
        反之如果 request 不會導致程式被 block 就是屬於 Asynchronous I/O
        https://docs.oracle.com/cd/E19683-01/806-4125/chap7rt-2/index.html
        oracle 這份文件列的這些即是