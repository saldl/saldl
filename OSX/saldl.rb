class Saldl < Formula
  desc "CLI downloader optimized for speed and early preview."
  homepage "https://saldl.github.io"
  url "https://github.com/saldl/saldl/archive/v32.tar.gz"
  sha256 "93a6f289d9520b986a4b5a6b4ab45ec97f890ae659cc06a2e391d35059cdfe8a"
  head "https://github.com/saldl/saldl.git", :shallow => false

  depends_on "pkg-config" => :build
  depends_on "asciidoc" => :build
  depends_on "docbook-xsl" => :build
  depends_on "libevent"

  if MacOS.version <= :mavericks
    # curl >= 7.42 is required
    depends_on "curl"
  else
    depends_on "curl" => :optional
  end

  def install
    # a2x/asciidoc needs this to build the man page successfully
    ENV["XML_CATALOG_FILES"] = "#{etc}/xml/catalog"

    args = ["--prefix=#{prefix}"]

    # head uses git describe to acquire a version
    args << "--saldl-version=v#{version}" unless build.head?

    system "./waf", "configure", *args
    system "./waf", "build"
    system "./waf", "install"
  end

  test do
    system "#{bin}/saldl", "http://brew.sh/index.html"
    assert File.exist? "index.html"
  end
end
