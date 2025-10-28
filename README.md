git init                     # Khởi tạo repo Git trong thư mục hiện tại
git clone <url>              # Sao chép repo từ GitHub
git clone <url> <folder>     # Clone về thư mục cụ thể

git status                   # Kiểm tra trạng thái file
git log                      # Xem lịch sử commit
git log --oneline --graph    # Log gọn, hiển thị nhánh dạng cây
git show <commit_id>         # Xem chi tiết 1 commit

git add <file>               # Thêm file cụ thể vào stage
git add .                    # Thêm tất cả file thay đổi
git commit -m "Nội dung commit"   # Commit thay đổi
git commit --amend           # Sửa commit gần nhất

git remote -v                # Kiểm tra remote hiện tại
git remote add origin <url>  # Thêm remote repo
git push -u origin main      # Push nhánh main lần đầu
git push                     # Push sau này
git pull                     # Kéo code mới về
git fetch                    # Lấy dữ liệu remote nhưng chưa merge

git branch                   # Liệt kê các nhánh
git branch <tên-nhánh>       # Tạo nhánh mới
git checkout <tên-nhánh>     # Chuyển sang nhánh
git checkout -b <tên-nhánh>  # Tạo và chuyển sang nhánh mới
git merge <tên-nhánh>        # Gộp nhánh khác vào nhánh hiện tại
git branch -d <tên-nhánh>    # Xóa nhánh đã merge

git diff                     # So sánh thay đổi
git diff <file>              # So sánh 1 file cụ thể
git restore <file>           # Hủy thay đổi chưa add
git restore --staged <file>  # Gỡ file khỏi stage

git merge <branch>           # Gộp nhánh (giữ lịch sử)
git rebase <branch>          # Gộp nhánh (làm thẳng lịch sử)
git rebase --continue        # Tiếp tục sau khi giải quyết conflict
git rebase --abort           # Hủy rebase

git reset --soft HEAD~1      # Quay lại commit trước, giữ thay đổi
git reset --hard HEAD~1      # Quay lại commit trước, xóa thay đổi
git revert <commit_id>       # Tạo commit đảo ngược 1 commit cũ
git checkout <commit_id>     # Xem lại phiên bản cũ (chế độ detached)

git submodule add <url> <path>   # Thêm submodule
git submodule update --init      # Khởi tạo submodule
git submodule update --remote    # Cập nhật submodule

git stash                        # Lưu tạm thay đổi
git stash pop                    # Lấy lại thay đổi đã stash
git tag <tên-tag>                # Đánh dấu phiên bản
git tag                          # Liệt kê tag
git clean -fd                    # Xóa file không được theo dõi

# Xử lý conflict
git status                   # Xem file đang conflict
# Mở file, sửa tay rồi:
git add <file>               
git rebase --continue hoặc git merge --continue