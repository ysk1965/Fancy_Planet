using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using System.IO;

static class Type
{
	public const int ANIMATOR = 0;
	public const int ANIMATION = 1;
	public const int NONE = 2;
}

static class Constants
{
    public const int SKINNEDMESH = 0;
	public const int RENDERMESH = 1;
    public const int LAYER = 2;
	public const int END = 3;
}

static class AnimationState
{
	public const int IDLE = 0;
	public const int JUMP = 1;
	public const int RUN = 2;
	public const int LOAD = 3;
	public const int SHOT = 4;
	public const int DEATH = 5;
}

static class AniType
{
	public const int NOT_ALL = 0; // 중간에 애니메이션 가능 (구현)
	public const int ALL = 1; // 애니메이션이 끝나야 다른 얘니메이션이 가능 (구현)
	public const int MODIFIABLE = 2; // 애니메이션 시간이 변경될수 있음
	public const int NOT_CYCLE = 3; // 한번만 재생되고 마지막 모습에서 정지
	public const int CAN_BACK_PLAY = 4; // 뒤로도 재생이 필요한 애니메이션(구현)
	public const int CYCLE_NEED_TIME = 5; // 싸이클이 필요하지만 처음과 끝이 같지 않는 애니메이션 
	public const int CONTINUOUS_PLAYBACK = 6; // 재생이 끊기지 않고, 다음 인덱스 애니메이션이 자동 실행
	public const int PLAY_NOW = 7;
}

public class FBX_Exporter : MonoBehaviour
{
    private static FileStream fs = new FileStream("NewAstronaut.bss", FileMode.Create);
    private BinaryWriter bw = new BinaryWriter(fs);
    private Animation ani;
    private Animator ani_T;
	private int num = new int();
	private int aninum = new int();
	SkinnedMeshRenderer[] smr;
	private int aniType = new int();

    void Start()
	{
		aniType = Type.ANIMATOR; // 애니메이션 타입

		smr = GetComponentsInChildren<SkinnedMeshRenderer>();
        num = 0;
        Transform t = GetComponent<Transform>();
        Made(t, bw);

		bw.Write(true); //애니메이션 유무
		
		aninum = -2;
		
		if(aniType == Type.ANIMATION)
			ani = GetComponent<Animation>();
		else if(aniType == Type.ANIMATOR)
			ani_T = GetComponent<Animator>();
	}
    private void Made(Transform mt, BinaryWriter bw)
    {
        Write(mt, bw);

        if (mt.childCount > 0)
        {
            bw.Write(true);
            bw.Write(mt.childCount);
        }
        else
            bw.Write(false);

        for (int i = 0; i < mt.childCount; i++)
        {
            Transform ct = mt.GetChild(i);
            Made(ct, bw);
        }
    }
    private void Write(Transform mt, BinaryWriter bw)
    {
        SkinnedMeshRenderer SkinnedMesh = mt.GetComponent<SkinnedMeshRenderer>();
        MeshRenderer MeshRender = mt.GetComponent<MeshRenderer>();

		if (SkinnedMesh != null)
		{
			bw.Write(Constants.SKINNEDMESH); // 

			Mesh objMesh = SkinnedMesh.sharedMesh;
			

			for (int sub = 0; sub < objMesh.subMeshCount; sub++)
			{
				bw.Write(objMesh.subMeshCount); // 서브 메쉬 갯수

				int num = new int();

				string[] strNme = { "Legacy Shaders/Diffuse", "Legacy Shaders/Lightmapped/Diffuse",
					"Legacy Shaders/Bumped Diffuse", "Legacy Shaders/Bumped Specular",
					"Legacy Shaders/Diffuse Detail" , "Legacy Shaders/Lightmapped/Bumped Specular", "Standard"};

				for (int nStr = 0; nStr < strNme.Length; ++nStr)
				{
					if (strNme[nStr] == SkinnedMesh.materials[sub].shader.name)
					{
						if (nStr == 0)
						{
							num = 1;
						}
						else if (nStr == 1 || nStr == 2 || nStr == 3)
						{
							num = 2;
						}
						else if (nStr == 5)
						{
							num = 3;
						}
						else if (nStr == 6)
						{
							num = 2;
						}
						else
						{
							num = 1;
						}
						break;
					}
				}

				bw.Write(num); // 그리기 타입
				
				int nVertices = objMesh.vertexCount;

				bw.Write(nVertices); // 정점의 갯수

				Vector3[] Vertices = objMesh.vertices;
				Vector2[] UV = objMesh.uv;
				Vector3[] Normals = objMesh.normals;
				Vector4[] Tangents = objMesh.tangents;
				BoneWeight[] Weights = objMesh.boneWeights;
								
				int[] Indices = objMesh.triangles;

				uint Indexstart = objMesh.GetIndexStart(sub);
				uint Indexcount = objMesh.GetIndexCount(sub);

				bw.Write(Indexcount); // 인덱스 갯수

				for (uint i = Indexstart; i < Indexstart + Indexcount; i++)
				{// 인덱스
					bw.Write(Indices[i]);
				}
				for (int i = 0; i < nVertices; i++)
				{
					bw.Write(Vertices[i].x); // 정점 좌표
					bw.Write(Vertices[i].y);
					bw.Write(Vertices[i].z);
				}
				for (int i = 0; i < nVertices; i++)
				{// uv값
					bw.Write(UV[i].x);
					bw.Write(UV[i].y);
				}
				if (num > 1)
				{
					for (int i = 0; i < nVertices; i++)
					{// 노말 정보
						bw.Write(Normals[i].x);
						bw.Write(Normals[i].y);
						bw.Write(Normals[i].z);
					}
					for (int i = 0; i < nVertices; i++)
					{// 탄젠트 정보
						bw.Write(Tangents[i].x);
						bw.Write(Tangents[i].y);
						bw.Write(Tangents[i].z);
					}
				}
				for (int i = 0; i < nVertices; i++)
				{// 뼈 가중치					
					bw.Write(Weights[i].weight0);
					bw.Write(Weights[i].weight1);
					bw.Write(Weights[i].weight2);					
				}
				for (int i = 0; i < nVertices; i++)
				{// 뼈 인덱스
					bw.Write(Weights[i].boneIndex0);
					bw.Write(Weights[i].boneIndex1);
					bw.Write(Weights[i].boneIndex2);
					bw.Write(Weights[i].boneIndex3);
				}
				

				bw.Write(objMesh.bindposes.Length); // 본 포즈 길이
				for (int i = 0; i < objMesh.bindposes.Length; i++)
				{
					bw.Write(objMesh.bindposes[i].m00);
					bw.Write(objMesh.bindposes[i].m01);
					bw.Write(objMesh.bindposes[i].m02);
					bw.Write(objMesh.bindposes[i].m03);

					bw.Write(objMesh.bindposes[i].m10);
					bw.Write(objMesh.bindposes[i].m11);
					bw.Write(objMesh.bindposes[i].m12);
					bw.Write(objMesh.bindposes[i].m13);

					bw.Write(objMesh.bindposes[i].m20);
					bw.Write(objMesh.bindposes[i].m21);
					bw.Write(objMesh.bindposes[i].m22);
					bw.Write(objMesh.bindposes[i].m23);

					bw.Write(objMesh.bindposes[i].m30);
					bw.Write(objMesh.bindposes[i].m31);
					bw.Write(objMesh.bindposes[i].m32);
					bw.Write(objMesh.bindposes[i].m33);
				}
				//}
				// 텍스쳐 파일 이름

				string str = SkinnedMesh.materials[sub].mainTexture.name;

				str += '\0';
				bw.Write(str.Length);
				for (int i = 0; i < str.Length; i++)
				{
					bw.Write(str[i]);
				}
				if (num > 1)
				{
					str = SkinnedMesh.materials[sub].mainTexture.name;
					str += '1';
					str += '\0';
					bw.Write(str.Length);
					for (int i = 0; i < str.Length; i++)
					{
						bw.Write(str[i]);
					}
				}
				if (num > 2)
				{
					str = SkinnedMesh.materials[sub].mainTexture.name;
					str += '2';
					str += '\0';
					bw.Write(str.Length);
					for (int i = 0; i < str.Length; i++)
					{
						bw.Write(str[i]);
					}
				}

				string objectName = mt.name;
				objectName += '\0';

				bw.Write(objectName.Length); //본 이름 길이
				for (int i = 0; i < objectName.Length; i++)
					bw.Write(objectName[i]); // 본 이름

				if (mt.name == "ScifiGrenadeLauncherStatic")
					bw.Write(1); // 총
				else if (mt.name == "Bip001 Spine1")
					bw.Write(2); // 허리
				else
					bw.Write(0);

				for (int boneNum = 0; boneNum < smr[0].bones.Length; boneNum++)
				{
					if (mt.name == smr[0].bones[boneNum].name)
					{
						bw.Write(boneNum); // 본 인덱스 
						break;
					}
					else if (boneNum == smr[0].bones.Length - 1)
						bw.Write(-1);
				}

			}
		}
		else if (MeshRender != null)
		{
			MeshFilter meshFilter = mt.GetComponent<MeshFilter>();

			bw.Write(Constants.RENDERMESH); // 

			Mesh objMesh = meshFilter.sharedMesh;
				

			for (int sub = 0; sub < objMesh.subMeshCount; sub++)
			{
				bw.Write(objMesh.subMeshCount); // 서브 메쉬 갯수

				string[] strNme = { "Legacy Shaders/Diffuse", "Legacy Shaders/Lightmapped/Diffuse",
					"Legacy Shaders/Bumped Diffuse", "Legacy Shaders/Bumped Specular",
					"Legacy Shaders/Diffuse Detail" , "Legacy Shaders/Lightmapped/Bumped Specular", "Standard", ""};

				for (int nStr = 0; nStr < strNme.Length; ++nStr)
				{
					if (strNme[nStr] == MeshRender.materials[sub].shader.name)
					{
						if (nStr == 0)
						{
							num = 1;
						}
						else if (nStr == 1 || nStr == 2 || nStr == 3)
						{
							num = 2;
						}
						else if (nStr == 5)
						{
							num = 3;
						}
						else if (nStr == 6)
						{
							num = 2;
						}
						else
						{
							num = 1;
						}
						break;
					}
				}

				bw.Write(num); // 그리기 타입

				int nVertices = objMesh.vertexCount;

				bw.Write(nVertices); // 정점의 갯수

				Vector3[] Vertices = objMesh.vertices;
				Vector2[] UV = objMesh.uv;
				Vector3[] Normals = objMesh.normals;
				Vector4[] Tangents = objMesh.tangents;
				BoneWeight[] Weights = objMesh.boneWeights;

				int[] Indices = objMesh.triangles;

				bw.Write(Indices.Length); // 인덱스 갯수
				for (int i = 0; i < Indices.Length; i++)
				{// 인덱스
					bw.Write(Indices[i]);
				}
				for (int i = 0; i < nVertices; i++)
				{
					bw.Write(Vertices[i].x); // 정점 좌표
					bw.Write(Vertices[i].y);
					bw.Write(Vertices[i].z);
				}
				for (int i = 0; i < nVertices; i++)
				{// uv값
					bw.Write(UV[i].x);
					bw.Write(UV[i].y);
				}
				if (num > 1)
				{
					for (int i = 0; i < nVertices; i++)
					{// 노말 정보
						bw.Write(Normals[i].x);
						bw.Write(Normals[i].y);
						bw.Write(Normals[i].z);
					}
					for (int i = 0; i < nVertices; i++)
					{// 탄젠트 정보
						bw.Write(Tangents[i].x);
						bw.Write(Tangents[i].y);
						bw.Write(Tangents[i].z);
					}
				}

				// 텍스쳐 파일 이름

				string str = MeshRender.materials[sub].mainTexture.name;

				str += '\0';
				bw.Write(str.Length);
				for (int i = 0; i < str.Length; i++)
				{
					bw.Write(str[i]);
				}
				if (num > 1)
				{
					str = MeshRender.materials[sub].mainTexture.name;
					str += '1';
					str += '\0';
					bw.Write(str.Length);
					for (int i = 0; i < str.Length; i++)
					{
						bw.Write(str[i]);
					}
				}
				if (num > 2)
				{
					str = MeshRender.materials[sub].mainTexture.name;
					str += '2';
					str += '\0';
					bw.Write(str.Length);
					for (int i = 0; i < str.Length; i++)
					{
						bw.Write(str[i]);
					}
				}

				if (aniType != Type.NONE)
				{
					bw.Write(mt.localScale.x);

					string objectName = mt.name;
					objectName += '\0';

					bw.Write(objectName.Length); //본 이름 길이
					for (int i = 0; i < objectName.Length; i++)
						bw.Write(objectName[i]); // 본 이름

					if (mt.name == "ScifiGrenadeLauncherStatic")
						bw.Write(1); // 총
					else if (mt.name == "Bip001 Spine1")
						bw.Write(2); // 허리
					else
						bw.Write(0);
				}


				if (aniType != Type.NONE)
				{
					for (int boneNum = 0; boneNum < smr[0].bones.Length; boneNum++)
					{
						if (mt.name == smr[0].bones[boneNum].name)
						{
							bw.Write(boneNum); // 본 인덱스 
							break;
						}
						else if (boneNum == smr[0].bones.Length - 1)
							bw.Write(-1);
					}
				}
			}
		}
		else
		{
			bw.Write(Constants.LAYER);


			string objectName = mt.name;
			objectName += '\0';

			bw.Write(objectName.Length); //본 이름 길이
			for (int i = 0; i < objectName.Length; i++)
				bw.Write(objectName[i]); // 본 이름

			if (mt.name == "ScifiGrenadeLauncherStatic")
				bw.Write(1); // 총
			else if (mt.name == "Bip001 Spine1")
				bw.Write(2); // 허리
			else
				bw.Write(0);

			for (int boneNum = 0; boneNum < smr[0].bones.Length; boneNum++)
			{
				if (mt.name == smr[0].bones[boneNum].name)
				{
					bw.Write(boneNum); // 본 인덱스 
					break;
				}
				else if (boneNum == smr[0].bones.Length - 1)
					bw.Write(-1);
			}
		}
		
	}
    private void AnimationWrite(Transform mt)
    {
		bw.Write(mt.localPosition.x);	

		if(mt.name == "Bip001")
			bw.Write(mt.localPosition.y - 1.1654701f);
		else
			bw.Write(mt.localPosition.y);

		bw.Write(mt.localPosition.z);

		bw.Write(mt.localRotation.x);
		bw.Write(mt.localRotation.y);
		bw.Write(mt.localRotation.z);
		bw.Write(mt.localRotation.w);
		
		for (int i = 0; i < mt.childCount; i++)
		{
			Transform ct = mt.GetChild(i);
			AnimationWrite(ct);
		}
	}

	// Update is called once per frame

	void Update()
	{
		Transform tr = GetComponent<Transform>();
		string[] aniName = { "idle", "walk", "왼쪽 걷기", "오른쪽 걷기", "발사", "점프1", "점프2", "점프3", "죽음", "댄스"}; // 애니메이션 이름
		float[] aniLength = { 9.26f, 1.0f, 1.0f, 1.0f, 1.3f, 0.27f, 1.03f, 0.25f, 3.07f,22.43f }; // 시간
		int[] length = {595, 61, 61, 61, 115, 27, 63, 23, 188, 1178};
		int[] type = { AniType.NOT_ALL, AniType.CAN_BACK_PLAY, AniType.NOT_ALL, AniType.NOT_ALL, AniType.ALL,
		AniType.ALL, AniType.NOT_ALL, AniType.PLAY_NOW,  AniType.NOT_CYCLE,  AniType.ALL};
		
		if (aniType == Type.ANIMATION)
		{
			if (aninum == -2)
			{
				aninum++;
				bw.Write(ani.GetClipCount()); // 애니메이션 수
				Transform[] sub = GetComponentsInChildren<Transform>();
				bw.Write(sub.Length);
				num = -9;
			}
			if (num < 2 * ani.clip.frameRate * aniLength[aninum + 1] + 1 && num > 0)
			{
				if (num == 1)
				{
					bw.Write((int)(2 * ani.clip.frameRate * aniLength[aninum + 1])); // 애니메이션 길이(프레임)
					bw.Write(aniLength[aninum + 1]); //시간
					bw.Write(type[aninum + 1]);
				}

				AnimationWrite(tr);
			}
			else if (num >= 2 * ani.clip.frameRate * aniLength[aninum + 1] + 1)
			{
				Debug.Log(aninum + "완료");

				aninum++;

				if (aninum == ani.GetClipCount() - 1)
				{
					bw.Close();
					return;
				}
				if (aninum < ani.GetClipCount() - 1)
					ani.PlayQueued(aniName[aninum], QueueMode.PlayNow);

				num = 0;
			}
			if (aninum < ani.GetClipCount() - 1)
				num++;
		}
		else if (aniType == Type.ANIMATOR)
		{
			if (aninum == -2)
			{
				aninum++;
				bw.Write(aniName.Length); // 애니메이션 수
				Transform[] sub = GetComponentsInChildren<Transform>();
				bw.Write(sub.Length);
				num = -9;
			}
			if (num < length[aninum + 1] + 1 && num > 0)
			{
				if (num == 1)
				{
					Debug.Log("시작");
					bw.Write((int)(length[aninum + 1] - 1)); // 애니메이션 길이(프레임)
					bw.Write(aniLength[aninum + 1]); //시간
					bw.Write(type[aninum + 1]);
				}

				AnimationWrite(tr);
			}
			else if (num >= length[aninum + 1] + 1)
			{
				Debug.Log("Next");
				aninum++;
				num = 0;
				int c = ani_T.GetInteger("c");
				ani_T.SetInteger("c", ++c);
			}
			if (ani_T.GetInteger("c") == aniLength.Length)
			{
				Debug.Log("완료");
				bw.Close();
			}
			num++;
		}
		else
		{
			bw.Close();
		}
	}
}