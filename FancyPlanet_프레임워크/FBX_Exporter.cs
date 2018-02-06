using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using System.IO;

static class Constants
{
    public const int MESH = 0;
    public const int LAYER = 1;
    public const int END = -1;
}

public class FBX_Exporter : MonoBehaviour
{

    void Start()
    {
        FileStream fs = new FileStream("SoldierModel.dat", FileMode.Create);
        BinaryWriter bw = new BinaryWriter(fs);
        Transform t = GetComponent<Transform>();
        Made(t, bw);
        bw.Write(Constants.END);
        bw.Close();
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
                // 텍스쳐 파일 이름

                string str = SkinnedMesh.materials[sub].mainTexture.name;
                Debug.Log(sub + " : " + str);
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

        Vector3 po = mt.position;
        bw.Write(po.x);
        bw.Write(po.y);
        bw.Write(po.z);
    }
    // Update is called once per frame
    void Update()
    {

    }
}
