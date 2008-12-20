famicom ROM cartridge utility - unagi
ROM dump script version 0.35.0
�����T�C�g http://unagi.sourceforge.jp/

--�X�N���v�g�t�@�C���� nes header �}�b�p�ԍ��ꗗ--
==Nintendo ���Ƃ��̌݊��i==
script        #
----------------
nrom.ud       0
unrom.ud      2
uorom.ud      2
gnrom.ud      66
mmc1normal.ud 1
mmc1_4M.ud    1
mmc2.ud       9
mmc3.ud       4,118,etc..
mmc4.ud       10
mmc5.ud       5

==Konami ���Ƃ��̌݊��i==
����: VRC2,4 �̓}�b�p�ԍ������܂���ɗ����܂���B
script        #
----------------
rc809.ud      87
vrc1.ud       75
vrc2a01.ud    23
vrc2a10.ud    22
vrc3.ud       73
vrc4a.ud      21
vrc4b.ud      25
vrc4c.ud      21
vrc4d.ud      25
vrc4e.ud      23
vrc6.ud       24,26
vrc7.ud       85

==Sunsoft ��==   ==Taito ��==     ==Irem ��==
script        #  script        #  script        #
---------------- ---------------- ----------------
sunsoft2b.ud  89 x1_005.ud     80 irem_g101.ud  32
sunsoft3.ud   67 x1_017.ud     82 irem_h3001.ud 65
sunsoft5b.ud  69 tc0190.ud     33

==Namcot ��==    ==Bandai ��==    ==Jaleco ��==
script        #  script        #  script        #
---------------- ---------------- ----------------
namcot118.ud  88 bandai_70.ud  70 jaleco_72.ud  72
namcot163.ud  19 fcg1.ud       16 jaleco_92.ud  92
                 lz93d50.ud    16
                 fcjump2.ud    16?

--�⑫����--
�⑫�����ɂĕK�v���얢�m�F�ƋL�ڂ������̂œ���m�F���Ƃꂽ���̂͐����
�������肢���܂��B

����̃\�t�g�͌ʂɃR�}���h���C���I�v�V����������K�v������܂��B
���̏ꍇ���L�̗v�f��ǉ����Ă��������B
 unagi.exe d [�X�N���v�g�t�@�C��] [�o�̓t�@�C��] [flag] [mapper]
mapper �̕����͂Ȃ��Ă��悢�ꍇ�������ł��B

==diskbios.ud==
disksystem �� bios ��p�ł��B���̃X�N���v�g�́ANES�w�b�_�𐶐����܂���B

==mmc1_4M.ud==
��e��ROM���ڂ̃t�@�C�i���t�@���^�W�[I.II�ƃh���S���N�G�X�gIV��p�ł��B

==mmc1normal.ud==
��L�ȊO�� MMC1 ���ڃ\�t�g�Ɏg�p���Ă��������B

==mmc3.ud==
flag:S mapper:118
	�����_���[�Y�t�����C�[�X
	RPG �l���Q�[��
flag:_ mapper:118
	�A���}�W��

==namcot106.ud==
flag:SV
	�t�@�~���[�T�[�L�b�g'91

==sunsoft3.ud==
�R�����g�ɂ���������ł����A�X�N���v�g�������I�������ɂ��̃}�b�p�̃J
�Z�b�g�������ĂȂ����ƂɋC�Â��܂����B���얢�m�F�ł��B

--Konami VRC series--
�}�b�p�ԍ��͂��ĂɂȂ�܂���B�Q�[���^�C�g���ƈ�v����X�N���v�g���g�p
���Ă��������Bvrc �V���[�Y���̗p���Ă��Ȃ����͖̂��֌W�ł��B

vrc1.ud:
	����΂�S�G�����I���炭�蓹��
	�L���O�R���O2 �{��̃��K�g���p���`
	�G�L�T�C�e�B���O�{�N�V���O
	�S�r�A�g��
vrc2a01.ud:
	�R�i�~ ���C���C���[���h
	�������`
	�h���S���X�N���[�� �S�肵����
	���l�� (��:����)
	������q�`�G
	����΂�S�G����2
vrc2a10.ud:
	����΂�y�i���g���[�X
	�c�C���r�[3 �|�R�|�R�喂��
vrc3.ud:
	�����֎�
vrc4a.ud:
	���C���C���[���h2 SOS!!�p�Z����
vrc4b.ud:
	�o�C�I�~���N�� �ڂ����ăE�p (��:ROM)
	����΂�S�G�����O�` �����������L�Z�� (�{���� VRC2)
	�O���f�B�E�XII
	���[�T�[�~�j�l�� �W���p���J�b�v
vrc4c.ud:
	����΂�S�G�����O�`2 �V���̍���
vrc4d.ud:
	Teenage Mutant Ninja Turtles
	Teenage Mutant Ninja Turtles 2
vrc4e.ud:
	�p���f�B�E�X���I
	�����邷�؂���� �ڂ��h���L��������
	�N���C�V�X�t�H�[�X
	�^�C�j�[�g�D�[���E�A�h�x���`���[�Y (��:����)
vrc6.ud:
	������`��
vrc6.ud: flag:S mapper:26
	�鲐�L MADARA
	�G�X�p�[�h���[��2
vrc7.ud:
	���O�����W���|�C���g
	�^�C�j�[�g�D�[���E�A�h�x���`���[�Y2

--�X�N���v�g��W--
150���炢����}�b�p�̃X�N���v�g��S�Ď������ŏ������Ƃ͏o���܂���B��
�y�ɃX�N���v�g��ǉ����邱�Ƃ��o����̂ŁA�X�N���v�g���������l�͌����T
�C�g�܂ŘA�������������B
�̗p��͉��L�Ƃ����Ă��������܂��B

* ���̃X�N���v�g���g���Ď��ۂɓ���m�F�����ēǂݏo��������(�Ƃ�������
  �����Ȃ��� sunsoft3.ud �͓��얢�m�F...)
* ����m�F�������\�t�g���{�̖��̂��^�Ԃ��X�N���v�g�擪�ɃR�����g������
  �邱��
* �ǂݏo���ɕK�v�ȃ��W�X�^�͏��������Ă���ǂݏo������ (�}�b�p�̃}�C
  �i�[�o�[�W�����ɂ���ēd��������̏����l���قȂ�ꍇ������̂ŕK��
  �s���Ă�������)

