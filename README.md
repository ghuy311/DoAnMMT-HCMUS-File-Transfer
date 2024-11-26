README
Giới thiệu
Dự án này triển khai mô hình client-server dựa trên giao thức TCP để tải xuống tệp từ máy chủ. Client có thể kết nối đến server, nhận danh sách các tệp có sẵn và tải xuống các tệp theo yêu cầu. Tất cả các tệp được tải về sẽ được lưu trữ trong thư mục output trên máy client, và hệ thống đảm bảo rằng các tệp đã tải xuống sẽ không được tải lại.

Hướng dẫn sử dụng
1. Cấu trúc thư mục
Server
Tất cả các tệp cần cung cấp cho client phải được lưu trong cùng thư mục với tệp thực thi Server.exe.
Client
Sau khi tải xuống thành công, các tệp sẽ được lưu tại thư mục output của client.
2. Cách khởi chạy chương trình
Khởi động Server:

Chạy Server.exe trước khi client bắt đầu kết nối.
Khởi động Client:

Sau khi server được khởi tạo, chạy chương trình client để kết nối.
Client sẽ tự động nhận danh sách các tệp khả dụng từ server và hiển thị.
3. Gửi yêu cầu tải xuống tệp
Client sử dụng tệp input.txt để gửi yêu cầu tải xuống các tệp từ server.
Cấu trúc của input.txt:
Ghi tên các tệp cần tải xuống (mỗi tệp trên một dòng).
Ví dụ:
Copy code
file1.txt
file2.jpg
Sau khi nhận yêu cầu, server sẽ xử lý và gửi tệp về client.
4. Cơ chế xử lý tệp tải xuống
Các tệp được tải về sẽ được lưu tại thư mục output trên máy client.
Hệ thống kiểm tra các tệp đã tải về:
Nếu tệp đã tồn tại trong thư mục output, client sẽ không tải lại để tránh trùng lặp và tiết kiệm băng thông.
Lưu ý
Đảm bảo cả server và client chạy trên cùng một mạng và sử dụng cùng cấu hình IP/Port.
Kiểm tra rằng thư mục output trên client có quyền ghi để lưu các tệp tải về.
