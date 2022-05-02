using System.Text;

if (args.Length == 0) {
    Console.Error.WriteLine("No operation specified.");
    Console.Error.WriteLine("\t--concat file1 file2 file3 ... fileN output");
    Console.Error.WriteLine("\t--extract file directory");
    return 1;
}

if (args.Length < 3) {
    Console.Error.WriteLine("Not enough arguments.");
    return 1;
}

return args[0] switch {
    "--concat" => ConcatFiles(args[1..^1], args[^1]),
    "--extract" => ExtractFiles(args[1], args[2]),
    _ => 1
};

static int ConcatFiles(string[] FilePaths, string OutputFile) {
    var _strings = FilePaths.Aggregate((acc, val) => acc + '\0' + val) + '\0';
    var _strarr = Encoding.Default.GetBytes(_strings);
    
    using var _strm = new FileStream(OutputFile, FileMode.Create);
    
    _strm.Write(BitConverter.GetBytes(_strarr.LongLength));
    _strm.Write(_strarr);

    foreach (var file in FilePaths) {
        using var _fs = new FileStream(file, FileMode.Open);
        _strm.Write(BitConverter.GetBytes(_fs.Length));
        _fs.CopyTo(_strm);
    }

    return 0;
}

static int ExtractFiles(string FilePath, string OutputDirectory) {
    using var _instrm = new FileStream(FilePath, FileMode.Open);
    Span<byte> _reusableInt64Buf = stackalloc byte[sizeof(long)];
    long _headerSize = -1;
    string[] _fileNames;

    {
        _instrm.Read(_reusableInt64Buf);
        _headerSize = BitConverter.ToInt64(_reusableInt64Buf);

        Span<byte> _strbuf = new byte[_headerSize];
        _instrm.Read(_strbuf);
        _fileNames = Encoding.Default.GetString(_strbuf).Split('\0', StringSplitOptions.RemoveEmptyEntries);
    }

    if (!Directory.Exists(OutputDirectory)) Directory.CreateDirectory(OutputDirectory);

    for (int i = 0; i < _fileNames.Length; i++) {
        _instrm.Read(_reusableInt64Buf);
        long _fileSize = BitConverter.ToInt64(_reusableInt64Buf);

        var _dstPath = Path.Combine(OutputDirectory, _fileNames[i]);
        var _dirPath = Path.GetDirectoryName(_dstPath) ?? "";
        if (!Directory.Exists(_dirPath)) Directory.CreateDirectory(_dirPath);
        using var _outStream = new FileStream(_dstPath, FileMode.Create);

        long _blockSize = 1000000;
        for (long blocks = _fileSize; blocks > 0; blocks -= _blockSize) {
            Span<byte> _databuf = new byte[blocks > _blockSize ? _blockSize : blocks];
            _instrm.Read(_databuf);
            _outStream.Write(_databuf);
            _databuf.Clear();
        }
    }

    return 0;
}