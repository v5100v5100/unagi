famicom ROM cartridge utility - unagi
ROM dump script version 0.26.2
�����T�C�g http://sourceforge.jp/projects/unagi/

--�͂��߂�--
�M�҂Ɏ茳�ɂ������J�Z�b�g��ǂݏo�����߂Ɏg�p���܂����B�⑫�������K�v
�ȕ��̐����ł��B���얢�m�F�ƋL�ڂ������̂œ���m�F���Ƃꂽ���̂͐����
�������肢���܂��B

����̃\�t�g�͌ʂɃR�}���h���C���I�v�V����������K�v������܂��B
���̏ꍇ���L�̗v�f��ǉ����Ă��������B
 unagi.exe d [�X�N���v�g�t�@�C��] [�o�̓t�@�C��] [flag] [mapper]
mapper �̕����͂Ȃ��Ă��悢�ꍇ�������ł��B

--------
diskbios.map
disksystem �� bios ��p�ł��B���̃X�N���v�g�́ANES�w�b�_�𐶐����܂���B

--------
mmc1_4M.map
��e��ROM���ڂ̃t�@�C�i���t�@���^�W�[I.II�ƃh���S���N�G�X�gIV��p�ł��B

--------
mmc1normal.map
��L�ȊO�� MMC1 ���ڃ\�t�g�Ɏg�p���Ă��������B

--------
mmc3.map #3, �o���A���g
flag:S mapper:118
	�����_���[�Y�t�����C�[�X
	RPG �l���Q�[��
flag:_ mapper:118
	�A���}�W��

--------
namcot106.map #19
flag:SV
	�t�@�~���[�T�[�L�b�g'91

--------
rc809.map #87
�O�[�j�[�Y����, �n�C�p�[�I�����s�b�N�a�l��, �W�����R�����\�t�g�p�B
�O�[�j�[�Y�̓o���G�[�V�������ٗl�ɖL�x�Ȃ̂ł��ԈႦ�Ȃ��悤�B

--------
sunsoft3.map #67
�R�����g�ɂ���������ł����A�X�N���v�g�������I�������ɂ��̃}�b�p�̃J
�Z�b�g�������ĂȂ����ƂɋC�Â��܂����B���얢�m�F�ł��B

--------
vrc7.map
���O�����W���|�C���g�̂ݓ���m�F�ς݁B�L�����N�^ROM���ڂ� Tiny Toon 
Adventures 2 ���얢�m�F�B

--Konami VRC series--
iNES �̒�`�͌��ʂƂ��Ă��Ȃ肸����ŁA�}�b�p�ԍ��͂��ĂɂȂ�܂���B
�Q�[���^�C�g���ƈ�v����X�N���v�g���g�p���Ă��������B
vrc �V���[�Y���̗p���Ă��Ȃ����͖̂��֌W�ł��B

vrc1.map:
	����΂�S�G�����I���炭�蓹��
	�L���O�R���O2 �{��̃��K�g���p���`
	�G�L�T�C�e�B���O�{�N�V���O
	�S�r�A�g��
vrc2a01.map:
	�R�i�~ ���C���C���[���h
	�������`
	�h���S���X�N���[�� �S�肵���� (���m�F)
	���l�� (��:����)
	������q�`�G
	����΂�S�G����2
vrc2a10.map:
	����΂�y�i���g���[�X
	�c�C���r�[3 �|�R�|�R�喂��
vrc3.map:
	�����֎�
vrc4a.map:
	���C���C���[���h2 SOS!!�p�Z����
vrc4b.map:
	�o�C�I�~���N�� �ڂ����ăE�p (��:ROM)
	����΂�S�G�����O�` �����������L�Z��
	�O���f�B�E�XII
	���[�T�[�~�j�l�� �W���p���J�b�v
vrc4c.map:
	����΂�S�G�����O�`2 �V���̍���
vrc4d.map:
	Teenage Mutant Ninja Turtles
	Teenage Mutant Ninja Turtles 2
vrc4e.map:
	�p���f�B�E�X���I
	�����邷�؂���� �ڂ��h���L��������
	�N���C�V�X�t�H�[�X
	�^�C�j�[�g�D�[���E�A�h�x���`���[�Y (��:����)
vrc6.map:
	������`��
vrc6.map: flag:S mapper:26
	�鲐�L MADARA
	�G�X�p�[�h���[��2
vrc7.map:
	���O�����W���|�C���g
	�^�C�j�[�g�D�[���E�A�h�x���`���[�Y2 (���얢�m�F�ŕʃX�N���v�g�̉\����)

--�X�N���v�g��W--
150���炢����}�b�p�̃X�N���v�g��S�Ď������ŏ������Ƃ͏o���܂���B��
�y�ɃX�N���v�g��ǉ����邱�Ƃ��o����̂ŁA�X�N���v�g���������l�͌����T
�C�g�܂ŘA�������������B
�̗p��͉��L�Ƃ����Ă��������܂��B

* ���̃X�N���v�g���g���Ď��ۂɓ���m�F�����ēǂݏo��������(�Ƃ�������
  �����Ȃ��� sunsoft3.map �͓��얢�m�F...)
* ����m�F�������\�t�g���{�̖��̂��^�Ԃ��X�N���v�g�擪�ɃR�����g������
  �邱��
* �ǂݏo���ɕK�v�ȃ��W�X�^�͏��������Ă���ǂݏo������ (�}�b�p�̃}�C
  �i�[�o�[�W�����ɂ���ēd��������̏����l���قȂ�ꍇ������̂ŕK��
  �s���Ă�������)

