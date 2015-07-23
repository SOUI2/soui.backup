#pragma once

#include <unknown/obj-ref-i.h>

namespace SOUI
{
    interface  IDataSetObserver : public IObjRef{
        /**
        * This method is called when the entire data set has changed,
        * most likely through a call to {@link Cursor#requery()} on a {@link Cursor}.
        */
        virtual void onChanged()  PURE;

        /**
        * This method is called when the entire data becomes invalid,
        * most likely through a call to {@link Cursor#deactivate()} or {@link Cursor#close()} on a
        * {@link Cursor}.
        */
        virtual void onInvalidated()  PURE;
    };

    interface IAdapter : public IObjRef{
        /**
        * Register an observer that is called when changes happen to the data used by this adapter.
        *
        * @param observer the object that gets notified when the data set changes.
        */
        virtual void registerDataSetObserver(IDataSetObserver * observer) PURE;

        /**
        * Unregister an observer that has previously been registered with this
        * adapter via {@link #registerDataSetObserver}.
        *
        * @param observer the object to unregister.
        */
        virtual void unregisterDataSetObserver(IDataSetObserver * observer) PURE;

        /**
        * How many items are in the data set represented by this Adapter.
        * 
        * @return Count of items.
        */
        virtual int getCount() PURE;   


        /**
        * Get a View that displays the data at the specified position in the data set. You can either
        * create a View manually or inflate it from an XML layout file. When the View is inflated, the
        * parent View (GridView, ListView...) will apply default layout parameters unless you use
        * {@link android.view.LayoutInflater#inflate(int, android.view.ViewGroup, boolean)}
        * to specify a root view and to prevent attachment to the root.
        * 
        * @param position The position of the item within the adapter's data set of the item whose view
        *        we want.
        * @param pItem The old view to reuse, if possible. Note: You should check that this view
        *        is non-null and of an appropriate type before using. If it is not possible to convert
        *        this view to display the correct data, this method can create a new view.
        *        Heterogeneous lists can specify their number of view types, so that this View is
        *        always of the right type (see {@link #getViewTypeCount()} and
        *        {@link #getItemViewType(int)}).
        */
        virtual void getView(int position, SWindow * pItem) PURE;


        /**
        * Get the type of View that will be created by {@link #getView} for the specified item.
        * 
        * @param position The position of the item within the adapter's data set whose view type we
        *        want.
        * @return An integer representing the type of View. Two views should share the same type if one
        *         can be converted to the other in {@link #getView}. Note: Integers must be in the
        *         range 0 to {@link #getViewTypeCount} - 1. {@link #IGNORE_ITEM_VIEW_TYPE} can
        *         also be returned.
        * @see #IGNORE_ITEM_VIEW_TYPE
        */
        virtual int getItemViewType(int position) PURE;

        /**
        * <p>
        * Returns the number of types of Views that will be created by
        * {@link #getView}. Each type represents a set of views that can be
        * converted in {@link #getView}. If the adapter always returns the same
        * type of View for all items, this method should return 1.
        * </p>
        * <p>
        * This method will only be called when when the adapter is set on the
        * the {@link AdapterView}.
        * </p>
        * 
        * @return The number of types of Views that will be created by this adapter
        */
        virtual int getViewTypeCount() PURE;

        /**
        * @return true if this adapter doesn't contain any data.  This is used to determine
        * whether the empty view should be displayed.  A typical implementation will return
        * getCount() == 0 but since getCount() includes the headers and footers, specialized
        * adapters might want a different behavior.
        */
        virtual bool isEmpty() PURE;
    };
    
}