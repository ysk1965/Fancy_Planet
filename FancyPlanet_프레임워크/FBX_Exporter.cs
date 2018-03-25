using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using System.IO;

static class Constants
{
    public const int MESH = 0;
    public const int LAYER = 1;
    public const int END = 3;
}

public class FBX_Exporter : MonoBehaviour
{
    private static FileStream fs = new FileStream("Test_WhaleModel_local.dat", FileMode.Create);
    private BinaryWriter bw = new BinaryWriter(fs);
    private Animation ani;
    private int num = new int();
	private int aninum = new int();
	SkinnedMeshRenderer[] smr;
    void Start()
    {
        smr = GetComponentsInChildren<SkinnedMeshRenderer>();
        num = 0;
        ani = GetComponent<Animation>();
        Transform t = GetComponent<Transform>();
        Made(t, bw);

		aninum = -2;

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

        if (SkinnedMesh != null)
        {
            bw.Write(Constants.MESH); // 

            Mesh objMesh = SkinnedMesh.sharedMesh;
            int[] indices;
            int p1, p2, p3;
            int num = new int();
            List<Vector3> m = new List<Vector3>();

            objMesh.GetVertices(m);

            string[] strNme = { "Legacy Shaders/Diffuse", "Legacy Shaders/Lightmapped/Diffuse",
                    "Legacy Shaders/Bumped Diffuse", "Legacy Shaders/Bumped Specular",
                    "Legacy Shaders/Diffuse Detail" , "Legacy Shaders/Lightmapped/Bumped Specular"};

            for (int nStr = 0; nStr < strNme.Length; ++nStr)
            {
                if (strNme[nStr] == SkinnedMesh.material.shader.name)
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
                    else
                    {
                        num = 1;
                    }
                    break;
                }
            }

            for (int sub = 0; sub < objMesh.subMeshCount; sub++)
            {
                bw.Write(objMesh.subMeshCount); // 서브 메쉬 갯수

                bw.Write(num); // 그리기 타입

                indices = objMesh.GetIndices(sub);

                bw.Write(indices.Length); // 정점의 갯수

                for (int i = 0; i < indices.Length; i += 3)
                {
                    p1 = indices[i + 0];
                    p2 = indices[i + 1];
                    p3 = indices[i + 2];

                    bw.Write(m[p1].x); // 정점 좌표
                    bw.Write(m[p1].y);
                    bw.Write(m[p1].z);
                    bw.Write(m[p2].x);
                    bw.Write(m[p2].y);
                    bw.Write(m[p2].z);
                    bw.Write(m[p3].x);
                    bw.Write(m[p3].y);
                    bw.Write(m[p3].z);
                }
                for (int i = 0; i < indices.Length; i += 3)
                {// uv값
                    p1 = indices[i + 0];
                    p2 = indices[i + 1];
                    p3 = indices[i + 2];
                    bw.Write(objMesh.uv[p1].x);
                    bw.Write(objMesh.uv[p1].y);
                    bw.Write(objMesh.uv[p2].x);
                    bw.Write(objMesh.uv[p2].y);
                    bw.Write(objMesh.uv[p3].x);
                    bw.Write(objMesh.uv[p3].y);
                }
                if (num > 1)
                {
                    for (int i = 0; i < indices.Length; i += 3)
                    {// 노말 정보
                        p1 = indices[i + 0];
                        p2 = indices[i + 1];
                        p3 = indices[i + 2];
                        bw.Write(objMesh.normals[p1].x);
                        bw.Write(objMesh.normals[p1].y);
                        bw.Write(objMesh.normals[p1].z);
                        bw.Write(objMesh.normals[p2].x);
                        bw.Write(objMesh.normals[p2].y);
                        bw.Write(objMesh.normals[p2].z);
                        bw.Write(objMesh.normals[p3].x);
                        bw.Write(objMesh.normals[p3].y);
                        bw.Write(objMesh.normals[p3].z);
                    }
                    for (int i = 0; i < indices.Length; i += 3)
                    {// 탄젠트 정보
                        p1 = indices[i + 0];
                        p2 = indices[i + 1];
                        p3 = indices[i + 2];
                        bw.Write(objMesh.tangents[p1].x);
                        bw.Write(objMesh.tangents[p1].y);
                        bw.Write(objMesh.tangents[p1].z);
                        bw.Write(objMesh.tangents[p2].x);
                        bw.Write(objMesh.tangents[p2].y);
                        bw.Write(objMesh.tangents[p2].z);
                        bw.Write(objMesh.tangents[p3].x);
                        bw.Write(objMesh.tangents[p3].y);
                        bw.Write(objMesh.tangents[p3].z);
                    }
                }
                for (int i = 0; i < indices.Length; i += 3)
                {// 뼈 가중치
                    p1 = indices[i + 0];
                    p2 = indices[i + 1];
                    p3 = indices[i + 2];
                    bw.Write(objMesh.boneWeights[p1].weight0);
                    bw.Write(objMesh.boneWeights[p1].weight1);
                    bw.Write(objMesh.boneWeights[p1].weight2);
					
					bw.Write(objMesh.boneWeights[p2].weight0);
                    bw.Write(objMesh.boneWeights[p2].weight1);
                    bw.Write(objMesh.boneWeights[p2].weight2);

                    bw.Write(objMesh.boneWeights[p3].weight0);
                    bw.Write(objMesh.boneWeights[p3].weight1);
                    bw.Write(objMesh.boneWeights[p3].weight2);
                }
                for (int i = 0; i < indices.Length; i += 3)
                {// 뼈 인덱스
                    p1 = indices[i + 0];
                    p2 = indices[i + 1];
                    p3 = indices[i + 2];
                    bw.Write(objMesh.boneWeights[p1].boneIndex0);
                    bw.Write(objMesh.boneWeights[p1].boneIndex1);
                    bw.Write(objMesh.boneWeights[p1].boneIndex2);
                    bw.Write(objMesh.boneWeights[p1].boneIndex3);

                    bw.Write(objMesh.boneWeights[p2].boneIndex0);
                    bw.Write(objMesh.boneWeights[p2].boneIndex1);
                    bw.Write(objMesh.boneWeights[p2].boneIndex2);
                    bw.Write(objMesh.boneWeights[p2].boneIndex3);

                    bw.Write(objMesh.boneWeights[p3].boneIndex0);
                    bw.Write(objMesh.boneWeights[p3].boneIndex1);
                    bw.Write(objMesh.boneWeights[p3].boneIndex2);
                    bw.Write(objMesh.boneWeights[p3].boneIndex3);
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
            }
        }
        else
        {
            bw.Write(Constants.LAYER);
        }
		
		string objectName = mt.name;
		objectName += '\0';

		bw.Write(objectName.Length); //본 이름 길이
		for(int i=0;i< objectName.Length; i++)
			bw.Write(objectName[i]); // 본 이름

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
    private void AnimationWrite(Transform mt)
    {
        bw.Write(mt.localPosition.x);
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
		string[] aniName = { "death", "swim", "fastswim2", "dive" }; // 애니메이션 이름
		float[] aniLength = {2.7f ,23.333f, 5.367f, 2.7f, 9.33f }; // 시간
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
			}

			AnimationWrite(tr);
		}
		else if (num >= 2 * ani.clip.frameRate * ani.clip.length + 1)
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
}