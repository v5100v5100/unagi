famicom ROM cartridge utility - unagi
RAM read/write script version 0.7.1
�����T�C�g http://sourceforge.jp/projects/unagi/

--�͂��߂�--
�J�[�g���b�W�����ɑ��݂���g�� RAM ��ǂݏ������邽�߂̃X�N���v�g�ł��B
�g�� RAM �̎�ȗ��p���@�̓Q�[���̃Z�[�u�f�[�^��ۑ�����o�b�e���[�o�b
�N�A�b�v RAM �ł��B���̃X�N���v�g�𗘗p���邱�Ƃɂ���Ď��@�ƃG�~��
���[�^�ł̃Z�[�u�f�[�^�̑��݉^�p���\�ɂȂ�܂��B

--RAM read/write script ���L�̖��--
�g�� RAM �̓��e�͓ǂݏo���Ȃ��Ă��G�~�����[�^�ł̓���ɕK�v���Ȃ�����
�ƁA�G�~�����[�^�ł͏�ɃA�N�Z�X�\�ɂ���Γ����Ă��܂��܂��B
��L�ɉ����ARAM �̎d�l�� ROM �}�b�p�������ł��e�ʂ�A�N�Z�X���@���΂�
�΂�ł��邱�Ƃ�����AROM �Ɣ�ׂĉ�͂��i��łȂ�����Ƃ����̂��Ή���
�͂��߂Ă���̎����ł��B

--RAM �C���[�W�̎�舵���ɂ���--
�G�~�����[�^�Ŏg�p����� .sav �t�@�C���𗘗p���܂��B
�J�[�g���b�W�����̑�؂ȃf�[�^������ď������Ȃ��悤�ɒ��ӂ��ė��p����
���������B�f�[�^�����g���u���ɂ��� unagi development team �͐ӔC����
��܂���B

--�X�N���v�g�̕��@�ɂ���--
ROM dump script �p�b�P�[�W�ɕt������ syntax.txt ���������������B

--�X�N���v�g�ʏڍ�--
==namcot_fc91.map==
�t�@�~���[�T�[�L�b�g91��p�ł��B�G�~�����[�^�ō쐬����� .sav �t�@�C��
�̃T�C�Y�� 0x2000 byte �ł����A���m�ɂ� 0x800 byte �ł��B���̂��߁A
�Z�[�u�f�[�^�𗘗p����ꍇ�͉��L�̉��H���K�v�ł��B
offset        ���@->emu/ emu->���@
0x0000-0x17ff �����f�[�^�𖄂߂�/�f�[�^�����
0x1800-0x1fff �L���ȃZ�[�u�f�[�^�̈�

==normal.map==
��ʓI�� RAM �A�N�Z�X�f�[�^�ł��BMMC4, VRC2, VRC4, VRC6 �Ɏg���܂��B

==rc832.map==
�O���f�B�E�XII�̃J�[�g���b�W������ RAM �ɃA�N�Z�X���邾���̃X�N���v�g
�ŁA�Z�[�u�f�[�^�ł͂���܂���B

==snrom.map==
MMC1 �𗘗p������ʓI�ȃJ�[�g���b�W�p�X�N���v�g�ł��B

==sxrom.map==
MMC1 �𗘗p�����傫���J�[�g���b�W�p�X�N���v�g�ł��B�t�@�C�i���t�@���^
�W�[I.II�ł̂ݓ���m�F�����Ă��܂����A���h�̃Q�[���ŗ��p�ł���Ǝv����
���B
==tsrom.map==
MMC3 �𗘗p������ʓI�ȃJ�[�g���b�W�p�X�N���v�g�ł��B

==vrc7.map==
���O�����W���|�C���g��p�ł��B

--���������̂̃A�N�Z�X���@���s���ȃJ�Z�b�g--
�i���R, �L���O�I�u�L���O�X
�^�C�g�[, �~�l���o�g���T�[�K
