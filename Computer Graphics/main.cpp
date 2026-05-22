#include "scene.h"
#include "binary/animation.h"
#include "binary/skeleton.h"
#include "binary/player.h"

Shader* Scene::vertexShader = nullptr;
Shader* Scene::fragmentShader = nullptr;
Program* Scene::program = nullptr;
Camera* Scene::camera = nullptr;
Object* Scene::player = nullptr;
Texture* Scene::diffuse = nullptr;
Material* Scene::material = nullptr;
Object* Scene::lineDraw = nullptr;
Texture* Scene::lineColor = nullptr;
Material* Scene::lineMaterial = nullptr;

bool Scene::upperFlag = true;
bool Scene::lowerFlag = true;

void Scene::setup(AAssetManager* aAssetManager) {
    Asset::setManager(aAssetManager);

    Scene::vertexShader = new Shader(GL_VERTEX_SHADER, "vertex.glsl");
    Scene::fragmentShader = new Shader(GL_FRAGMENT_SHADER, "fragment.glsl");

    Scene::program = new Program(Scene::vertexShader, Scene::fragmentShader);

    Scene::camera = new Camera(Scene::program);
    Scene::camera->eye = vec3(0.0f, 0.0f, 80.0f);

    Scene::diffuse = new Texture(Scene::program, 0, "textureDiff", playerTexels, playerSize);
    Scene::material = new Material(Scene::program, diffuse);
    Scene::player = new Object(program, material, playerVertices, playerIndices);
    player->worldMat = scale(vec3(1.0f / 3.0f));

    Scene::lineColor = new Texture(Scene::program, 0, "textureDiff", {{0xFF, 0x00, 0x00}}, 1);
    Scene::lineMaterial = new Material(Scene::program, lineColor);
    Scene::lineDraw = new Object(program, lineMaterial, {{}}, {{}}, GL_LINES);
}

void Scene::screen(int width, int height) {
    Scene::camera->aspect = (float) width/height;
}

void Scene::update(float deltaTime) {
    Scene::program->use();
    Scene::camera->update();

    static float utime = 0, ltime = 0;
    utime += deltaTime;
    ltime += deltaTime;
    float usavetime, lsavetime;
    int uindex = int(utime) % 4, lindex = int(ltime) % 4;
    int unext_index = (uindex + 1) % 4, lnext_index = (lindex + 1) % 4;
    float uin_between_weight = utime - int(utime), lin_between_weight = ltime - int(ltime), in_between_weight;

    if(upperFlag)
        usavetime = utime;
    else
        utime = usavetime;

    if(lowerFlag)
        lsavetime = ltime;
    else
        ltime = lsavetime;

    mat4 m_id[28];
    for(int i = 0; i < 28; i++) {
        m_id[i] = mat4(1.0f);
        int j = i;
        while(j != -1) {
            m_id[i] = translate(jOffsets[j]) * m_id[i];
            j = jParents[j];
        }
    }

    mat4 m_ia[28], ml1, ml2;
    static mat4 m_temp[28];
    for(int i = 0; i < 28; i++) {
        m_ia[i] = mat4(1.0f);
        int j = i;
        while (j != -1) {
            if(j < 12) {
                ml1 = rotate(radians(motions[lindex][j * 3 + 5]), vec3(0.0f, 0.0f, 1.0f))
                           * rotate(radians(motions[lindex][j * 3 + 3]), vec3(1.0f, 0.0f, 0.0f))
                           * rotate(radians(motions[lindex][j * 3 + 4]), vec3(0.0f, 1.0f, 0.0f));

                ml2 = rotate(radians(motions[lnext_index][j * 3 + 5]), vec3(0.0f, 0.0f, 1.0f))
                           *rotate(radians(motions[lnext_index][j * 3 + 3]), vec3(1.0f, 0.0f, 0.0f))
                           *rotate(radians(motions[lnext_index][j * 3 + 4]), vec3(0.0f, 1.0f, 0.0f));
                in_between_weight = lin_between_weight;
            }
            else {
                ml1 = rotate(radians(motions[uindex][j * 3 + 5]), vec3(0.0f, 0.0f, 1.0f))
                           * rotate(radians(motions[uindex][j * 3 + 3]), vec3(1.0f, 0.0f, 0.0f))
                           * rotate(radians(motions[uindex][j * 3 + 4]), vec3(0.0f, 1.0f, 0.0f));

                ml2 = rotate(radians(motions[unext_index][j * 3 + 5]), vec3(0.0f, 0.0f, 1.0f))
                           *rotate(radians(motions[unext_index][j * 3 + 3]), vec3(1.0f, 0.0f, 0.0f))
                           *rotate(radians(motions[unext_index][j * 3 + 4]), vec3(0.0f, 1.0f, 0.0f));
                in_between_weight = uin_between_weight;
            }
            if(lowerFlag && upperFlag) {
                m_temp[j] = mat4_cast(slerp(quat_cast(ml1), quat_cast(ml2), in_between_weight));
                m_ia[i] = translate(jOffsets[j]) * m_temp[j] * m_ia[i];
            }
            if(!lowerFlag && upperFlag){
                if(j < 12)
                    m_ia[i] = translate(jOffsets[j]) * m_temp[j] * m_ia[i];
                else {
                    m_temp[j] = mat4_cast(slerp(quat_cast(ml1), quat_cast(ml2), in_between_weight));
                    m_ia[i] = translate(jOffsets[j]) * m_temp[j] * m_ia[i];
                }
            }
            if(lowerFlag && !upperFlag){
                if(j >= 12)
                    m_ia[i] = translate(jOffsets[j]) * m_temp[j] * m_ia[i];
                else {
                    m_temp[j] = mat4_cast(slerp(quat_cast(ml1), quat_cast(ml2), in_between_weight));
                    m_ia[i] = translate(jOffsets[j]) * m_temp[j] * m_ia[i];
                }
            }
            if(!lowerFlag && !upperFlag)
                m_ia[i] = translate(jOffsets[j]) * m_temp[j] * m_ia[i];
            j = jParents[j];
        }
    }

    vector<Vertex> myplayerVertices;
    for(int i = 0; i < playerVertices.size(); i++){
        mat4 mi = mat4(0.0f);
        for(int j = 0; j < 4; j++) {
            int bone = playerVertices[i].bone[j];
            if (bone != -1)
                mi += playerVertices[i].weight[j] * m_ia[bone] * inverse(m_id[bone]);
        }
        myplayerVertices.push_back({mi * vec4(playerVertices[i].pos, 1.0f), mi * vec4(playerVertices[i].nor, 1.0f),
                               playerVertices[i].tex, playerVertices[i].bone, playerVertices[i].weight});
    }
    // Line Drawer
    //glLineWidth(20);
    //Scene::lineDraw->load({{vec3(-20.0f, 0.0f, 0.0f)}, {vec3(20.0f, 0.0f, 0.0f)}}, {0, 1});
    //Scene::lineDraw->draw();

    Scene::player->load(myplayerVertices, playerIndices);
    Scene::player->draw();
}

void Scene::setUpperFlag(bool flag)
{
    Scene::upperFlag = flag;
}

void Scene::setLowerFlag(bool flag)
{
    Scene::lowerFlag = flag;
}
