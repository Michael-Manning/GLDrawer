﻿using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using GLDrawerCLR;

namespace GLDrawer
{
    public abstract class GameObject : IDisposable
    {
        public Transform transform = new Transform();
        private GameObject iparent = null; //allows retrieval of parent
        public GameObject Parent
        {
            get => iparent;
            set
            {
                //should signal to old reference first
                if (iparent != null)
                    iparent.GOchildren.Remove(this);
                internalGO.setParent(value.internalGO);
                iparent = value;

                value.GOchildren.Add(this);

                if (DrawIndex == 0)
                    DrawIndex = value.DrawIndex;
            }
        }
        public float DeltaTime => Canvas.DeltaTime;
        public float Time => Canvas.Time;
        public void ClearParent()
        {
            internalGO.clearParent();
            iparent = null;
        }
        public int DrawIndex { get => internalGO.drawIndex; set => internalGO.drawIndex = value; }
        public GameObject Instantiate(GameObject original) => Canvas.Instantiate(original);
        public GameObject Instantiate(GameObject original, vec2 position, float rotation = 0) => Canvas.Instantiate(original, position, rotation);
        public GameObject Instantiate(GameObject original, vec2 position, float rotation, GameObject parent) => Canvas.Instantiate(original, position, rotation, parent);
        public bool IsClone { get; private set; } = false;
        public GameObject Clone()
        {
            GameObject clone = (GameObject) this.MemberwiseClone();
            clone.transform = new Transform(transform.Position, transform.Rotation, transform.Velocity);
            clone.ClearParent();
            clone.ClearChildren();
            clone.internalGO = new unmanaged_GO(); //prevent garbage collector from collecting twice
            clone.IsClone = true;
            return clone;
        }
        internal GLCanvas can;
        public GLCanvas Canvas
        {
            get
            {
                if (can == null)
                    throw new NullReferenceException("Canvas wall null / gameObject no added to a canvas");
                return can;
            }     
        }
        private List<Shape> shapeChildren = new List<Shape>();
        protected Shape AddChildShape(Shape s)
        {
            CheckCan();
            can.Add(s);
            shapeChildren.Add(s);
            s.Parent = this;
            if (s.DrawIndex == 0)
                s.DrawIndex = DrawIndex;
            return s;
        }
        private List<GameObject> GOchildren = new List<GameObject>();
        protected GameObject AddChildGameObject(GameObject g)
        {
            CheckCan();
            can.Add(g);
            g.Parent = this;
            return g;
        }

        private void ClearChildren()
        {
            GOchildren.RemoveAll(g => g.Parent != this);
            GOchildren. ForEach(o => o.ClearParent());
            GOchildren.Clear();

            shapeChildren.RemoveAll(g => g.Parent != this);
            shapeChildren.ForEach(o => o.ClearParent());
            shapeChildren.Clear();
        }

        private void KillChildren(bool destroy = false)
        {
            if (destroy)
            {
                //circles back, but runs through only once
                Destroy();
                return;
            }

            GOchildren.RemoveAll(g => g.Parent != this);
            GOchildren.ForEach(o => o.KillChildren(true));
            GOchildren.Clear();

            shapeChildren.RemoveAll(g => g.Parent != this);
            shapeChildren.ForEach(o => o.ClearParent());
            // shapeChildren.ForEach(o => can.RemoveShape(o));
            foreach (Shape shape in shapeChildren)
            {
                can.Remove(shape);
              //  can.disposeBuffer.Add(shape.Dispose);
            }
            shapeChildren.Clear();
        }

        private bool destroyed = false;
        public void Destroy(float time = 0, bool killChildren = true)
        { 
            Canvas.Invoke(delegate
            {
                if (destroyed)
                      return;
                destroyed = true;
                if (killChildren)
                    KillChildren();
                else
                    ClearChildren();

                Canvas.EarlyUpdate -= InternalEarlyUpdate;
                Canvas.Update -= InternalUpdate;
                Canvas.LateUpdate -= InternalLateUpdate;

                ClearChildren();
                Canvas.Remove(this);
            }, time);
        }


        internal void InternalEarlyUpdate()
        {
            if (!startFlag)
            {
                startFlag = true;
                Start();
            }
            EarlyUpdate();
        }
        internal void InternalUpdate()
        {
            Update();
        }
        internal void InternalLateUpdate()
        {
            LateUpdate();
            transform.Position += transform.Velocity;
            internalGO.position = new unmanaged_vec2(transform.Position.x, transform.Position.y);
            internalGO.angle = transform.Rotation;
        }


        private void CheckCan()
        {
            if (Canvas == null || !Canvas.initialized)
                throw new NullReferenceException("No canvas set, null canvas, or canvas was uninitialized. Gameobject contructors are strongly unadvised");
        }
        private bool startFlag = false; //has start been run already

        public virtual void Start() { }
        public virtual void EarlyUpdate() { }
        public virtual void Update() { }
        public virtual void LateUpdate() { }

        internal unmanaged_GO internalGO = new unmanaged_GO();

        private bool disposed = false;
        public void Dispose()
        {
            Dispose(true);
            GC.SuppressFinalize(this);
        }
        private void Dispose(bool disposing)
        {
            if (!disposed)
            {
                disposed = true;
                internalGO.dispose();
                if (disposing)
                {
                    //if any managed resources need to be disposed, it should be done here
                    ClearChildren();
                }
            }
        }
        ~GameObject()
        {
            if (IsClone)
                Console.WriteLine("Collecting Clone");
            Dispose(false);
        }

    }
    /// <summary>
    ///represents a gameObject's location in world space
    /// </summary>
    /// <param>name= "position" location in world space</param>
    /// <param name="velocity">Gets multiplied by DeltaTime and added to the position every frame</param>
    /// <param name="maxAngularVelocity">Speed cap of the transform at any angle. OVERRIDDIN BY RIGIDBODIES </param>
    public class Transform
    {
        public vec2 Position;
        public vec2 Velocity;
        public float Rotation;

        public Transform()
        {
            Position = vec2.Zero; 
            Rotation = 0;
        }
        public Transform(vec2 position, float rotation = 0)
        {
            Position = position;
            Rotation = rotation;
        }
        public Transform(vec2 position, float rotation, vec2 velocity) : this(position, rotation)
        {
            Velocity = velocity;
        }
    }
}
