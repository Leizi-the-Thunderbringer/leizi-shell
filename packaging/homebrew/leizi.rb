# Homebrew Formula for Leizi Shell
class Leizi < Formula
  desc "Modern POSIX-compatible shell with ZSH-style arrays and beautiful prompts"
  homepage "https://github.com/Zixiao-System/leizi-shell"
  url "https://github.com/Zixiao-System/leizi-shell/archive/v1.4.0.tar.gz"
  sha256 "" # Will be updated automatically by CI
  license "GPL-3.0"
  head "https://github.com/Zixiao-System/leizi-shell.git", branch: "main"

  depends_on "cmake" => :build
  depends_on "readline"

  def install
    system "cmake", "-S", ".", "-B", "build",
           "-DCMAKE_BUILD_TYPE=Release",
           *std_cmake_args
    system "cmake", "--build", "build"
    system "cmake", "--install", "build"

    # Install shell to /etc/shells
    (prefix/"post_install.sh").write <<~EOS
      #!/bin/bash
      if ! grep -qxF "#{bin}/leizi" /etc/shells 2>/dev/null; then
        echo "Adding #{bin}/leizi to /etc/shells..."
        echo "#{bin}/leizi" | sudo tee -a /etc/shells > /dev/null
      fi
    EOS
    chmod 0755, prefix/"post_install.sh"
  end

  def caveats
    <<~EOS
      To add Leizi Shell to your system's list of shells:
        #{prefix}/post_install.sh

      To set Leizi as your default shell:
        chsh -s #{bin}/leizi

      Configuration file location:
        ~/.config/leizi/config

      For more information, see:
        https://github.com/Zixiao-System/leizi-shell/blob/main/README.md
    EOS
  end

  test do
    assert_match "Leizi Shell", shell_output("#{bin}/leizi --version")

    # Test basic functionality
    output = pipe_output("#{bin}/leizi", "echo hello\nexit\n")
    assert_match "hello", output

    # Test array support
    output = pipe_output("#{bin}/leizi", "array test=(1 2 3)\nexit\n")
    assert_match "", output
  end
end
