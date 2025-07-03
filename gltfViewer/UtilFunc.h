#pragma once

// �t�@�C�������݂��邩���`�F�b�N����֐�
bool fileExists(const std::string& filename) {
    DWORD dwAttrib = GetFileAttributesA(filename.c_str());
    return (dwAttrib != INVALID_FILE_ATTRIBUTES && !(dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
}

// glTF�t�@�C���g���q�����؂���֐�
bool isValidGltfFile(const std::string& filename) {
    if (filename.length() < 5) return false;

    std::string extension = filename.substr(filename.length() - 5);
    std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);

    return (extension == ".gltf" || filename.substr(filename.length() - 4) == ".glb");
}

// �R�}���h���C����������������֐�
std::string processCommandLineArgs(int argc, char* argv[]) {
    std::cout << "glTF�r���[�A�[ - OpenGL�����_���[" << std::endl;
    std::cout << "�g�p���@: " << argv[0] << " <gltf�t�@�C���p�X>" << std::endl;
    std::cout << "�T�|�[�g�`��: .gltf, .glb" << std::endl;
    std::cout << "������@:" << std::endl;
    std::cout << "  ESC: �I��" << std::endl;
    std::cout << "  WASD: �J�����ړ� (�O�㍶�E)" << std::endl;
    std::cout << "  QE: �J�����㉺�ړ�" << std::endl;
    std::cout << "  �}�E�X���h���b�O: �J������]" << std::endl;
    std::cout << "  �}�E�X�z�C�[��: �Y�[�� (������)" << std::endl;
    std::cout << std::endl;

    // �����̐����`�F�b�N
    if (argc < 2) {
        std::cout << "glTF�t�@�C�����w�肳��Ă��܂���B�O�p�`�Ńf�����[�h�����s���܂��B" << std::endl;
        std::cout << "glTF�t�@�C����ǂݍ��ނɂ́A�t�@�C���p�X�������Ƃ��Ďw�肵�Ă��������B" << std::endl;
        return ""; // �󕶎���̓f�����[�h������
    }

    if (argc > 2) {
        std::cout << "�x��: �����̈������w�肳��܂����B�ŏ��̈�����glTF�t�@�C���p�X�Ƃ��Ďg�p���܂��B" << std::endl;
    }

    std::string gltfFilePath = argv[1];
    std::cout << "glTF�t�@�C���̓ǂݍ��݂����s��: " << gltfFilePath << std::endl;

    // �t�@�C���g���q������
    if (!isValidGltfFile(gltfFilePath)) {
        std::cerr << "�G���[: �����ȃt�@�C���g���q�ł��B.gltf�܂���.glb�t�@�C�����K�v�ł��B" << std::endl;
        std::cerr << "�w�肳�ꂽ�t�@�C��: " << gltfFilePath << std::endl;
        return ""; // �f�����[�h�ő��s���邽�ߋ󕶎����Ԃ�
    }

    // �t�@�C�������݂��邩�`�F�b�N
    if (!fileExists(gltfFilePath)) {
        std::cerr << "�G���[: �t�@�C�������݂��Ȃ����A�A�N�Z�X�ł��܂���B" << std::endl;
        std::cerr << "�t�@�C���p�X: " << gltfFilePath << std::endl;

        // �𗧂G���[����񋟂��鎎��
        size_t lastSlash = gltfFilePath.find_last_of("\\/");
        if (lastSlash != std::string::npos) {
            std::string directory = gltfFilePath.substr(0, lastSlash);
            std::string filename = gltfFilePath.substr(lastSlash + 1);
            std::cerr << "�f�B���N�g��: " << directory << std::endl;
            std::cerr << "�t�@�C����: " << filename << std::endl;
        }

        return ""; // �f�����[�h�ő��s���邽�ߋ󕶎����Ԃ�
    }

    // �ǉ����؂̂��߃t�@�C���T�C�Y���擾
    HANDLE hFile = CreateFileA(gltfFilePath.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (hFile != INVALID_HANDLE_VALUE) {
        LARGE_INTEGER fileSize;
        if (GetFileSizeEx(hFile, &fileSize)) {
            std::cout << "�t�@�C���T�C�Y: " << fileSize.QuadPart << " �o�C�g" << std::endl;

            if (fileSize.QuadPart == 0) {
                std::cerr << "�G���[: �t�@�C������ł��B" << std::endl;
                CloseHandle(hFile);
                return "";
            }

            if (fileSize.QuadPart > 100 * 1024 * 1024) { // ���S�̂���100MB����
                std::cout << "�x��: �傫�ȃt�@�C�������o����܂����i>100MB�j�B�ǂݍ��݂Ɏ��Ԃ�������ꍇ������܂��B" << std::endl;
            }
        }
        CloseHandle(hFile);
    }

    std::cout << "�t�@�C�����؂��������܂����BglTF�t�@�C���̓ǂݍ��ݏ��������B" << std::endl;
    return gltfFilePath;
}